# Architecture

## 1. Design goals

1. **Small enough to audit.** A multicopter stack with one job per module.
2. **PX4-compatible seams.** Anyone who knows PX4 can find the sensor path,
   commander, navigator, attitude/position controllers, allocator, MAVLink and
   logger without learning a new architecture.
3. **No hidden work in the control loop.** All inter-module data moves through
   typed uORB topics. No module calls another module's control function
   directly.
4. **Deterministic scheduling.** Fixed-priority preemptive FreeRTOS tasks with
   documented rates, priorities and stack budgets.
5. **Failsafe by construction.** Loss of a sensor, link or mission is an
   explicit state, not an exception.

## 2. Layers

```
┌──────────────────────────────────────────────────────────────────────┐
│ GCS / Mission Planner            on-board shell                      │
│        │                              │                              │
│        ▼                              ▼                              │
│  modules/mavlink               systemcmds/                           │
├──────────────────────────────────────────────────────────────────────┤
│ Flight modules                                                      │
│  sensors → state_estimator → flight_mode_manager → mc_pos_control   │
│                                     │                    │          │
│                                     ▼                    ▼          │
│                              navigator           mc_att_control     │
│                                                          │          │
│                                                          ▼          │
│                                                 control_allocator   │
├──────────────────────────────────────────────────────────────────────┤
│ Middleware                                                          │
│  uORB pub/sub · params · time · storage · logging · work queues     │
├──────────────────────────────────────────────────────────────────────┤
│ Drivers                                                             │
│  IMU · baro · mag · GNSS · RC · PWM/DShot · telemetry · battery · SD│
├──────────────────────────────────────────────────────────────────────┤
│ HAL interfaces (src/hal)                                            │
├──────────────────────────────────────────────────────────────────────┤
│ Board support package (boards/thejas32)                             │
├──────────────────────────────────────────────────────────────────────┤
│ freertrtos adaptation layer (os/freertrtos)                         │
├──────────────────────────────────────────────────────────────────────┤
│ FreeRTOS kernel + RISC-V port (third_party)                         │
└──────────────────────────────────────────────────────────────────────┘
```

Dependencies point **downward only**. A driver never includes a flight module;
a module never pokes a register directly.

## 3. PX4 mapping

| PX4 | Freertrtos | Notes |
|---|---|---|
| `msg/*.msg` | `src/msg/*.msg` | Same idea: source of truth for topic layouts |
| `uORB` | `src/middleware/uorb` | Last-value topics + blocking wait, no dynamic allocation |
| `src/drivers/*` | `src/drivers/*` | One folder per device class |
| `src/modules/sensors` | `src/modules/sensors` | Aggregation, calibration, timestamping |
| `ekf2` | `src/modules/state_estimator` | Attitude + local/global position |
| `commander` | `src/modules/commander` | Arming, mode state, failsafe, home |
| `flight_mode_manager` | `src/modules/flight_mode_manager` | Mode classes produce setpoints |
| `mc_pos_control` | `src/modules/mc_pos_control` | Position → attitude setpoint |
| `mc_att_control` | `src/modules/mc_att_control` | Attitude → rate → torque |
| `control_allocator` | `src/modules/control_allocator` | Torque/thrust → motors |
| `navigator` | `src/modules/navigator` | Mission, RTL, waypoint sequencing |
| `mavlink` | `src/modules/mavlink` | Telemetry, commands, mission protocol |
| `logger` | `src/modules/logger` | Binary flight logs |
| `ROMFS/.../rcS` | `config/boards/*/init.d/rcS` | Plain-text startup script |
| `boards/*` | `boards/*` | BSP + linker + pin map |
| NuttX | `os/freertrtos` | Thin FreeRTOS adaptation layer |

## 4. Control data flow

1. **Acquisition.** `sensors` reads IMU/baro/mag/GNSS/RC/battery through
   drivers and publishes `sensor_imu`, `sensor_baro`, `sensor_mag`,
   `sensor_gps`, `rc_channels`, `battery_status`.
2. **Estimation.** `state_estimator` fuses IMU, baro, mag and GNSS. It
   publishes `vehicle_attitude`, `vehicle_angular_velocity`,
   `vehicle_local_position`, `vehicle_global_position` and
   `estimator_status`.
3. **Mode selection.** `commander` owns arming and the safety state machine.
   `flight_mode_manager` selects the active mode and writes
   `trajectory_setpoint` (position modes) or `vehicle_rates_setpoint`
   (manual mode).
4. **Position control.** `mc_pos_control` converts `trajectory_setpoint` +
   local position/velocity into `vehicle_attitude_setpoint` at 100 Hz.
5. **Attitude/rate control.** `mc_att_control` converts attitude setpoints
   into body rates at 400 Hz, then a 1 kHz rate loop produces
   `vehicle_torque_setpoint` and `vehicle_thrust_setpoint`.
6. **Allocation.** `control_allocator` maps torque/thrust to four motor
   commands using the airframe mixer, with saturation and slew limits.
7. **Actuation.** `pwm_out`/`dshot` writes the four ESC outputs. Arming is
   gated by `actuator_armed`.

The loop is broken only by uORB, so each stage can be unit-tested, logged and
replayed independently.

## 5. Navigation and mission flow

```
GCS mission upload ──► mavlink_mission ──► lib/mission (validate + store)
                                                   │
                                                   ▼
                                          navigator_mission
                                                   │
                     mission_current / position_setpoint
                                                   │
                                                   ▼
                                          flight_mode_manager
```

`navigator` owns the mission cursor. `flight_mode_manager` only consumes the
guidance output; it does not parse MAVLink or touch storage. RTL is a navigator
mode with its own climb/cruise/descend state machine.

## 6. Boot sequence

1. `startup.S` sets the stack, copies `.data`, zeroes `.bss`, calls `main`.
2. `board_init()` configures clocks, pins, UART, SPI/I2C, PWM and ADC.
3. `freertrtos_init()` creates kernel objects (queues, mutexes, timer).
4. `params_init()` loads defaults and the flash parameter file.
5. `uorb_init()` creates the static topic registry.
6. `storage_mount()` mounts the SD/flash filesystem.
7. `task_registry_start_all()` creates tasks in priority order.
8. `vTaskStartScheduler()` starts the scheduler.
9. `rcS` (or the compiled-in init table) starts modules in dependency order:
   sensors → estimator → commander → flight modes → controllers → allocator →
   navigator → mavlink → logger → watchdog.

The watchdog is started **before** any flight task so a failed task creation is
still observable.

## 7. Concurrency rules

- A module may only block on its own uORB subscriptions, queues or timers.
- No module waits for another module to finish; all coupling is data.
- uORB copies in and out under a short critical section. Never hold the topic
  lock while doing math or I/O.
- Drivers may sleep in their own task; the control loop never sleeps.
- The SD card write path is isolated in `logger` so a slow card cannot stall
  the estimator or controllers.
- All shared configuration is read-only after init except parameter writes,
  which are serialised by the params mutex.

## 8. Memory policy

- Static allocation for tasks, queues, uORB topics and the parameter table.
- `heap_4` is present for third-party libraries only; flight code must not call
  `malloc` in a periodic path.
- Topic payloads are fixed-size and copied; no pointers cross task boundaries.
- Stack sizes are budgeted per task in `task_registry.c` and checked by
  `configCHECK_FOR_STACK_OVERFLOW`.

## 9. Failure containment

| Failure | Containment |
|---|---|
| IMU stuck / invalid | `estimator_status` invalid → commander failsafe |
| GNSS loss | Dead-reckon for a bounded time, then RTL/land |
| GCS loss | Continue mission or RTL after timeout (configurable) |
| RC loss | RTL or land after timeout |
| Low battery | Warning → RTL → land at critical threshold |
| Mission abort | Hold, then RTL/land |
| Task overrun | Watchdog marks failsafe / reboots |
| SD card full | Logger stops, flight continues, `log_status` warns |
| Motor saturation | Allocator limits and reports; attitude loop prioritised |
