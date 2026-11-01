# Task schedule

FreeRTOS uses **higher numeric priority = higher urgency**. The table below is
the contract between modules and the scheduler. Values live in
`src/main/task_registry.c`; this document explains the reasoning.

## 1. Periodic tasks

| Task | Rate | Priority | Stack (words) | Deadline | Notes |
|---|---:|---:|---:|---:|---|
| `task_watchdog` | 10 Hz | 24 | 512 | 100 ms | Checks heartbeats; must never starve |
| `sensors` | 1 kHz IMU, 50 Hz baro/mag | 20 | 2048 | 1 ms per IMU sample | SPI/I2C reads, timestamping, publish |
| `state_estimator` | 400 Hz | 19 | 3072 | 2.5 ms | Attitude + position fusion |
| `mc_rate_control` | 1 kHz | 18 | 2048 | 1 ms | Could be merged into `mc_att_control` task |
| `mc_att_control` | 400 Hz | 17 | 2048 | 2.5 ms | Quaternion attitude loop |
| `mc_pos_control` | 100 Hz | 12 | 2048 | 10 ms | Position/velocity/altitude loop |
| `flight_mode_manager` | 100 Hz | 11 | 2048 | 10 ms | Mode logic, trajectory setpoint |
| `commander` | 20 Hz | 10 | 2048 | 50 ms | Arming, failsafe, state machine |
| `navigator` | 20 Hz | 9 | 2048 | 50 ms | Mission/RTL guidance |
| `rc_update` | 50 Hz | 8 | 1024 | 20 ms | RC decode + loss detection |
| `mavlink` | 50 Hz | 7 | 4096 | 20 ms | Telemetry, command parser |
| `land_detector` | 20 Hz | 6 | 1024 | 50 ms | Landing/auto-disarm assist |
| `logger` | 100 Hz | 5 | 4096 | 10 ms | Buffered SD writes |
| `battery_status` | 1 Hz | 4 | 1024 | 1 s | Filtering, remaining estimate |

## 2. Event-driven work

| Work | Mechanism | Priority | Notes |
|---|---|---|---|
| Parameter write | `params` mutex + flash work item | low | Serialised, may block the caller |
| Mission upload | `mavlink` task → `mission_storage` | low | Never blocks the control loop |
| Log flush | Ring buffer → SD write | low | Drop records rather than stall |
| Shell command | `systemcmds` task | low | Diagnostic only; can be disabled in flight |
| Watchdog kick | Each task calls `task_watchdog_checkin()` | — | Must be called from the task itself |

## 3. Scheduling rules

1. **Rate-monotonic priority assignment.** Shorter periods get higher
   priorities. The table follows this rule.
2. **No priority inversion on the control path.** uORB topic locks are held for
   a copy only; no module holds a lock while calling a driver or doing float
   math.
3. **One control loop per task.** `mc_att_control` may run the rate loop inside
   the same task at 1 kHz if the CPU budget allows; otherwise split it as
   shown.
4. **Deadline overrun is a fault.** If `state_estimator` or a controller misses
   its deadline repeatedly, `task_watchdog` sets a failsafe flag.
5. **Logging never back-pressures flight code.** The logger uses a ring buffer
   and increments `log_status.dropped_records` when full.
6. **MAVLink is not real-time.** It may be delayed by a slow UART or GCS; it
   must not own data needed by the controllers. The controllers read uORB
   snapshots.
7. **Interrupt handlers do the minimum.** ISRs post to a queue or set a
   notification; decoding and publishing happen in the owning task.

## 4. Stack budgeting

Stack sizes are in **words** (4 bytes on RV32), not bytes. The values above
include margin for `printf`-style formatting in diagnostic paths. Reduce them
only after measuring high-water marks with
`uxTaskGetStackHighWaterMark()` over a full mission.

## 5. CPU budget (target)

At 100 MHz RV32 with software float, a rough budget for a 1 kHz loop:

| Stage | Budget |
|---|---:|
| IMU read + publish | 150 µs |
| Attitude/position estimate | 400 µs |
| Attitude + rate control | 200 µs |
| Allocation + PWM write | 100 µs |
| Margin | 150 µs |

If the target has no FPU, prefer fixed-point or single-precision float and
measure early. The scaffold uses `float` for clarity; porting to a soft-float
MCU may require a fixed-point math layer.
