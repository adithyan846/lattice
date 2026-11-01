# Freertrtos Autopilot

A FreeRTOS-based multicopter autopilot scaffold, laid out after studying the
PX4-Autopilot source tree. The operating-system adaptation layer is named
**freertrtos** (the spelling requested for this project).

> **Status: scaffold, not flight-ready.** This repository defines the file
> tree, interfaces, task boundaries, configuration format and build wiring.
> Most flight logic is intentionally left as `TODO` stubs. Do not fly this
> code until every module has an implementation, unit tests, SITL/HIL coverage
> and a bench + flight test campaign.

---

## 1. What this project is

A small, understandable multicopter stack with the same architectural seams as
PX4:

| PX4 concept | This project | Purpose |
|---|---|---|
| `msg/` + uORB | `src/msg/` + `src/middleware/uorb/` | Typed, asynchronous data bus |
| `src/drivers/` | `src/drivers/` | IMU, baro, mag, GNSS, RC, ESC, telemetry, battery, SD |
| `src/modules/sensors` | `src/modules/sensors/` | Sensor acquisition and calibration |
| `src/modules/ekf2` | `src/modules/state_estimator/` | Attitude and position estimation |
| `src/modules/commander` | `src/modules/commander/` | Arming, mode state machine, failsafe |
| `src/modules/flight_mode_manager` | `src/modules/flight_mode_manager/` | Manual, stabilized, alt-hold, pos-hold, mission, RTL |
| `src/modules/mc_att_control` | `src/modules/mc_att_control/` | Attitude and rate control |
| `src/modules/mc_pos_control` | `src/modules/mc_pos_control/` | Position/velocity/altitude control |
| `src/modules/control_allocator` | `src/modules/control_allocator/` | Torque/thrust to motor mixer |
| `src/modules/navigator` | `src/modules/navigator/` | Waypoint mission and RTL guidance |
| `src/modules/mavlink` | `src/modules/mavlink/` | GCS telemetry, commands, mission protocol |
| `src/modules/logger` | `src/modules/logger/` | Flight and mission logging |
| `ROMFS/.../init.d/rcS` | `config/boards/*/init.d/rcS` | Boot task startup script |
| `boards/` | `boards/` | Board support packages |
| NuttX | **freertrtos** (`os/freertrtos/`) | FreeRTOS kernel adaptation layer |

The data flow is deliberately boring and explicit:

```
sensors ──► state_estimator ──► flight_mode_manager ──► mc_pos_control
   │                │                    │                    │
   │                │                    ▼                    ▼
   │                │            trajectory_setpoint   vehicle_attitude_setpoint
   │                │                                         │
   │                ▼                                         ▼
   └────────► vehicle_status / battery / GNSS        mc_att_control
                                                             │
                                                             ▼
                                                    vehicle_torque_setpoint
                                                    vehicle_thrust_setpoint
                                                             │
                                                             ▼
                                                    control_allocator
                                                             │
                                                             ▼
                                                    actuator_motors ──► ESC/PWM
```

MAVLink is a peer of the flight modules, never in the control loop:

```
GCS ◄──► mavlink ◄──► uORB topics ◄──► commander / navigator / logger
```

---

## 2. Requested capabilities and where they live

| # | Requirement | Primary files |
|---|---|---|
| 1 | Read/process onboard flight-control sensors | `src/drivers/imu/`, `barometer/`, `magnetometer/`, `src/modules/sensors/`, `state_estimator/` |
| 2 | GNSS positioning and navigation | `src/drivers/gnss/`, `src/modules/state_estimator/`, `src/modules/navigator/`, `src/lib/geo/` |
| 3 | ESC and motor control | `src/drivers/pwm_out/`, `src/modules/control_allocator/`, `src/modules/mc_att_control/`, `mc_pos_control/` |
| 4 | Stable attitude control | `src/modules/mc_att_control/`, `state_estimator/`, `src/lib/math/` |
| 5a | Arm / disarm | `src/modules/commander/arming.*`, `commander_state.*`, `config/boards/thejas32/init.d/rcS` |
| 5b | Manual / assisted stabilized mode | `src/modules/flight_mode_manager/modes/mode_manual.*`, `mode_stabilized.*`, `src/modules/rc_update/` |
| 5c | Position hold | `modes/mode_pos_hold.*`, `src/modules/mc_pos_control/position_controller.*` |
| 5d | Altitude hold | `modes/mode_alt_hold.*`, `src/modules/mc_pos_control/` |
| 5e | Autonomous mission | `modes/mode_mission.*`, `src/modules/navigator/navigator_mission.*`, `src/lib/mission/` |
| 5f | Return-to-home / launch | `modes/mode_rtl.*`, `src/modules/navigator/navigator_rtl.*`, `src/lib/rtl/`, `commander/home_position.*` |
| 6 | Execute pre-loaded waypoint mission | `src/lib/mission/`, `src/modules/navigator/`, `config/missions/demo_mission.plan` |
| 7 | Telemetry to Mission Planner / GCS | `src/modules/mavlink/`, `src/drivers/telemetry/`, `config/mavlink/streams.conf` |
| 8 | Receive mission commands from GCS | `mavlink_mission.*`, `mavlink_commands.*`, `src/modules/navigator/` |
| 9 | GCS live status display | `mavlink_streams.*` (heartbeat, sys_status, position, attitude, battery, mission_current) |
| 10 | Failsafe: low battery, link loss, GNSS loss, abort | `commander/failsafe.*`, `battery_status/`, `rc_update/`, `mavlink/`, `state_estimator/`, `navigator/` |
| 11 | Log flight and mission data | `src/modules/logger/`, `log_format.h`, `config/log/logger.conf`, `tools/log2csv.py` |

Detailed traceability: [`docs/requirements-traceability.md`](docs/requirements-traceability.md).

---

## 3. Top-level layout

```
demo-filetree-lattice/
├── README.md                 ← you are here
├── CMakeLists.txt            ← firmware + host-test build entry point
├── Makefile                  ← convenience wrapper
├── Kconfig                   ← optional feature selection
├── boards/                   ← board support packages (BSPs)
│   ├── thejas32/             ← reference RISC-V / FreeRTOS target
│   └── sitl/                 ← software-in-the-loop board shim
├── cmake/                    ← toolchain and build helpers
├── config/                   ← airframes, params, missions, init scripts
├── docs/                     ← architecture and bring-up documentation
├── os/freertrtos/            ← FreeRTOS adaptation layer
├── src/
│   ├── main/                 ← boot and task registry
│   ├── hal/                  ← hardware abstraction interfaces
│   ├── drivers/              ← sensor/actuator/telemetry drivers
│   ├── middleware/           ← uORB, params, time, storage, logging
│   ├── lib/                  ← math, geo, mission, RTL, CRC, ring buffers
│   ├── modules/              ← flight-control and system modules
│   ├── msg/                  ← message definitions + generated structs
│   └── systemcmds/           ← on-board shell commands
├── tests/                    ← host unit tests, SITL, HIL
├── third_party/              ← upstream FreeRTOS, MAVLink, littlefs
└── tools/                    ← build, flash, message-gen, log tools
```

Full generated tree: [`docs/file-tree.md`](docs/file-tree.md).

---

## 4. Task model (FreeRTOS)

| Task | Rate | Priority | Stack (words) | Produces |
|---|---:|---:|---:|---|
| `task_watchdog` | 10 Hz | 24 | 512 | watchdog events |
| `sensors` | 1 kHz IMU / 50 Hz baro | 20 | 2048 | `sensor_imu`, `sensor_baro`, `sensor_mag`, `sensor_gps` |
| `state_estimator` | 400 Hz | 19 | 3072 | `vehicle_attitude`, `vehicle_local_position`, `vehicle_global_position` |
| `mc_rate_control` (inside `mc_att_control`) | 1 kHz | 18 | 2048 | `vehicle_torque_setpoint` |
| `mc_att_control` | 400 Hz | 17 | 2048 | `vehicle_rates_setpoint` |
| `mc_pos_control` | 100 Hz | 12 | 2048 | `vehicle_attitude_setpoint` |
| `commander` | 20 Hz | 10 | 2048 | `vehicle_status`, `actuator_armed`, `failsafe_flags` |
| `flight_mode_manager` | 100 Hz | 11 | 2048 | `trajectory_setpoint` |
| `navigator` | 20 Hz | 9 | 2048 | `mission_current`, guidance setpoints |
| `mavlink` | 50 Hz | 7 | 4096 | telemetry, command acks |
| `logger` | 100 Hz | 5 | 4096 | log records |
| `battery_status` | 1 Hz | 4 | 1024 | `battery_status` |
| `rc_update` | 50 Hz | 8 | 1024 | `manual_control_setpoint` |
| `land_detector` | 20 Hz | 6 | 1024 | `vehicle_land_detected` |

Priorities are configurable in `src/main/task_registry.c`. The watchdog task
must have the highest priority so it can still run when a flight task is stuck.
See [`docs/task-schedule.md`](docs/task-schedule.md).

---

## 5. Build

### Host unit tests (works without hardware)

```sh
cd /home/adithyan/demo-filetree-lattice
make test
```

### Firmware (requires FreeRTOS kernel + RISC-V toolchain)

```sh
# 1. Add the kernel and the THEJAS32 port (or point CMake at existing copies)
#    third_party/FreeRTOS-Kernel/          ← FreeRTOS-Kernel checkout
#    third_party/freertos-thejas32/        ← THEJAS32 port checkout

# 2. Build
make thejas32
```

The build intentionally fails early with a clear message if the kernel or the
board port is missing. See [`docs/build.md`](docs/build.md) for the exact
toolchain, memory map and flashing steps.

---

## 6. Configuration

- Parameters: `config/boards/thejas32/default.params`
- Boot script: `config/boards/thejas32/init.d/rcS`
- Airframe/mixer: `config/airframes/quad_x/`
- Mission: `config/missions/demo_mission.plan` (QGC plan) and
  `config/missions/demo_mission.waypoints` (MAVLink WPL 110)
- MAVLink stream rates: `config/mavlink/streams.conf`
- Logger topic list: `config/log/logger.conf`
- Failsafe policy: `config/failsafe/failsafe.conf`

Parameters use `NAME value` lines. The on-board parameter store is intentionally
small and flash-backed; see `src/middleware/params/`.

---

## 7. Safety notes

This scaffold is **not** a substitute for a certified flight controller. Before
any powered test:

1. Implement and review the arming checks in `commander/arming.c`.
2. Verify the mixer against the actual frame with props removed.
3. Confirm the failsafe matrix in [`docs/failsafe.md`](docs/failsafe.md).
4. Run SITL missions and HIL tests.
5. Fly with a tether or in a controlled area, with a hardware kill switch.

---

## 8. License / provenance

Architectural inspiration comes from PX4-Autopilot (BSD-3-Clause). This tree is
a new layout; no PX4 source files are copied. Choose and add a license before
distribution. See [`docs/architecture.md`](docs/architecture.md) for the mapping
back to PX4 concepts.
