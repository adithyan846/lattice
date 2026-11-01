# Requirements traceability

Status legend: **S** = scaffold/interface only · **P** = partial · **I** = implemented and host-tested.

Status reflects the scaffold as shipped: a row is **I** only when the logic
named in the requirement has a passing host test in `tests/host/`. Rows that
also need task integration, a driver, or SITL verification stay **S** or **P**.

The tree was created from the requirement list in the project brief. Every row
has a primary implementation location and a verification method. "Scaffold"
means the file, interface and task wiring exist; the flight algorithm is a
`TODO` and must not be treated as airworthy.

## 1. Onboard sensor acquisition and processing

| ID | Requirement | Implementation | Verification | Status |
|---|---|---|---|---|
| SENS-1 | Read IMU (accel + gyro) at flight-control rate | `src/drivers/imu/imu.h`, `icm42688p.c`, `src/modules/sensors/sensors.c` | Host driver mock; HIL with recorded IMU data | S |
| SENS-2 | Read barometer for altitude | `src/drivers/barometer/ms5611.c`, `src/modules/sensors/sensors.c`, `src/lib/atmosphere/` | Pressure sweep unit test; bench comparison | S |
| SENS-3 | Read magnetometer for heading | `src/drivers/magnetometer/ist8310.c`, `src/modules/sensors/sensor_calibration.c` | Compass calibration test; turntable | S |
| SENS-4 | Timestamp, calibrate and publish sensor data | `src/modules/sensors/sensor_aggregator.c`, `sensor_calibration.c` | `sensor_imu` topic replay | S |
| SENS-5 | Detect sensor failure / invalid data | `src/modules/state_estimator/state_estimator.c` (publishes `estimator_status`), `src/modules/commander/failsafe.c` | Fault-injection test | S |

## 2. GNSS positioning and navigation

| ID | Requirement | Implementation | Verification | Status |
|---|---|---|---|---|
| GNSS-1 | Parse UBX and NMEA on a UART | `src/drivers/gnss/gnss_ubx.c`, `gnss_nmea.c`, `gnss.c` | Recorded UBX/NMEA streams | S |
| GNSS-2 | Publish fix, satellites, HDOP/VDOP, velocity | `src/msg/sensor_gps.msg`, `src/drivers/gnss/gnss.c` | Unit test parser; live receiver | S |
| GNSS-3 | Fuse GNSS into local/global position | `src/modules/state_estimator/position_estimator.c`, `ekf/ekf_position.c` | SITL trajectory comparison | S |
| GNSS-4 | Convert global ↔ local NED for navigation | `src/lib/geo/geo.c` | Host unit test (`tests/host/test_geo.c`) | I |
| GNSS-5 | Detect GNSS loss and degrade safely | `src/modules/commander/failsafe.c`, `estimator_status` | Fault injection / RF shield box | S |

## 3. ESC and motor control

| ID | Requirement | Implementation | Verification | Status |
|---|---|---|---|---|
| ESC-1 | Generate 4 PWM/DShot outputs | `src/drivers/pwm_out/pwm_out.c`, `dshot.c` | Logic analyser; motor test with props removed | S |
| ESC-2 | Map torque/thrust to motor commands | `src/modules/control_allocator/control_allocator.c`, `mixer_quad_x.c` | Host mixer unit test; thrust stand | P |
| ESC-3 | Enforce output limits and slew rate | `src/modules/control_allocator/actuator_limits.c` | Unit test with saturation cases (`test_control.c`) | I |
| ESC-4 | Gate outputs on arming state | `src/modules/commander/arming.c`, `actuator_armed` | Bench arm/disarm test | S |
| ESC-5 | Optional ESC telemetry (voltage/current/temp) | `src/drivers/pwm_out/esc_telemetry.c` | UART/DShot telemetry capture | S |

## 4. Stable attitude control

| ID | Requirement | Implementation | Verification | Status |
|---|---|---|---|---|
| ATT-1 | Estimate attitude from IMU (+mag) | `src/modules/state_estimator/attitude_estimator.c`, `ekf/ekf_attitude.c` | Host replay; motion capture | S |
| ATT-2 | Quaternion attitude controller | `src/modules/mc_att_control/attitude_controller.c` | Host step-response test (`test_control.c`) | I |
| ATT-3 | Body-rate PID with anti-windup | `src/modules/mc_att_control/rate_controller.c`, `src/lib/math/pid.c` | Host PID tests (`test_pid.c`, `test_control.c`) | I |
| ATT-4 | 1 kHz rate loop, 400 Hz attitude loop | `src/main/task_registry.c`, `mc_att_control.c` | Timing trace / GPIO toggling | S |
| ATT-5 | Motor saturation handling | `control_allocator/actuator_limits.c` | Saturation injection test (`test_control.c`) | I |

## 5. Flight modes

| ID | Requirement | Implementation | Verification | Status |
|---|---|---|---|---|
| MODE-1 | Arm / disarm with pre-arm checks | `src/modules/commander/arming.c`, `commander_state.c`, `commander.c` | Bench + SITL command tests | S |
| MODE-2 | Manual mode (direct body rates) | `src/modules/flight_mode_manager/modes/mode_manual.c` | RC stick mapping test | S |
| MODE-3 | Assisted stabilized mode (attitude + altitude assist) | `modes/mode_stabilized.c`, `mc_att_control/` | SITL hover test | S |
| MODE-4 | Position hold | `modes/mode_pos_hold.c`, `mc_pos_control/position_controller.c` | SITL position step | P |
| MODE-5 | Altitude hold | `modes/mode_alt_hold.c`, `mc_pos_control/position_controller.c` | SITL altitude step | P |
| MODE-6 | Autonomous mission | `modes/mode_mission.c`, `navigator/navigator_mission.c`, `lib/mission/` | SITL waypoint mission | S |
| MODE-7 | Return-to-home / return-to-launch | `modes/mode_rtl.c`, `navigator/navigator_rtl.c`, `lib/rtl/` | SITL RTL from offset | S |
| MODE-8 | Mode transitions are explicit and safe | `src/modules/flight_mode_manager/flight_mode_manager.c`, `mode.h` | Transition matrix test | S |

## 6. Autonomous waypoint mission

| ID | Requirement | Implementation | Verification | Status |
|---|---|---|---|---|
| MISS-1 | Store a pre-loaded mission (up to 64 items) | `src/lib/mission/mission.c`, `src/modules/navigator/navigator_mission.c` | Host unit test (`test_mission.c`) | I |
| MISS-2 | Validate seq, coordinates, altitude, radius | `src/lib/mission/mission.c` | Host unit test | I |
| MISS-3 | Fly waypoints with acceptance radius and hold time | `src/modules/navigator/waypoint.c`, `navigator_mission.c` | SITL mission with wind; host test (`test_nav.c`) | P |
| MISS-4 | Publish `mission_current` for GCS and logger | `navigator_mission.c`, `src/msg/mission_current.msg` | MAVLink inspection | S |
| MISS-5 | Abort mission and hold/RTL/land | `commander/failsafe.c`, `navigator.c` | GCS abort command test | S |
| MISS-6 | Example QGC plan and WPL 110 file | `config/missions/demo_mission.plan`, `.waypoints` | Upload to SITL | S |

## 7. Telemetry to Mission Planner / GCS

| ID | Requirement | Implementation | Verification | Status |
|---|---|---|---|---|
| TLM-1 | MAVLink v2 framing over UART | `src/modules/mavlink/mavlink_transport.c`, `mavlink.c` | Host framing test (`test_mavlink.c`); loopback + pymavlink | P |
| TLM-2 | Heartbeat, sys_status, position, attitude, battery | `mavlink_streams.c`, `config/mavlink/streams.conf` | Mission Planner / QGC live display | S |
| TLM-3 | Mission progress display | `mission_current` stream | QGC mission view | S |
| TLM-4 | Link quality and loss detection | `telemetry_status`, `mavlink_receiver.c` | Cable-pull test | S |
| TLM-5 | Parameter read/write from GCS | `mavlink_parameters` (inside `mavlink_commands.c`) | QGC parameter editor | S |
| TLM-6 | Statustext events | `mavlink_send_statustext()` | GCS message console | S |

## 8. Receive mission commands from GCS

| ID | Requirement | Implementation | Verification | Status |
|---|---|---|---|---|
| CMD-1 | Handle MAVLink `COMMAND_LONG` (arm, disarm, set mode, RTL, land) | `mavlink_commands.c`, `commander.c` | pymavlink command test | S |
| CMD-2 | Handle `MISSION_COUNT` / `MISSION_ITEM_INT` / `MISSION_REQUEST` | `mavlink_mission.c`, `lib/mission/` | Upload a 5-item mission | S |
| CMD-3 | Acknowledge accepted/rejected commands | `vehicle_command_ack.msg`, `mavlink_commands.c` | GCS ack inspection | S |
| CMD-4 | Enforce target system/component and authorization | `mavlink_receiver.c`, `commander/arming.c` | Fuzz + wrong-target tests | S |
| CMD-5 | Apply mission without rebooting | `mavlink_mission.c` → `navigator` | Upload then start mission | S |

## 9. GCS live vehicle status

| ID | Requirement | Implementation | Verification | Status |
|---|---|---|---|---|
| GCS-1 | Location (global + local) | `mavlink_streams.c`, `vehicle_global_position`, `vehicle_local_position` | QGC map | S |
| GCS-2 | Flight mode and arming state | `vehicle_status`, `HEARTBEAT` custom_mode | QGC header | S |
| GCS-3 | Battery voltage/current/remaining | `battery_status`, `BATTERY_STATUS` | Bench PSU sweep | S |
| GCS-4 | Mission progress | `mission_current`, `MISSION_CURRENT` | QGC mission list | S |
| GCS-5 | Failsafe and warning text | `failsafe_flags`, `STATUSTEXT` | Fault injection | S |

## 10. Failsafe actions

| ID | Requirement | Implementation | Verification | Status |
|---|---|---|---|---|
| FSAFE-1 | Low battery warning → RTL → land | `commander/failsafe.c`, `battery_status/`, params `BAT_*` | PSU voltage ramp | S |
| FSAFE-2 | RC loss | `rc_update/rc_update.c`, `failsafe.c` | Transmitter-off test | S |
| FSAFE-3 | GCS/telemetry loss | `mavlink/`, `telemetry_status`, `failsafe.c` | Cable pull / RF attenuator | S |
| FSAFE-4 | GNSS loss / degraded fix | `state_estimator/`, `failsafe.c` | RF shield / GNSS simulator | S |
| FSAFE-5 | Mission abort | `navigator.c`, `failsafe.c` | GCS abort command | S |
| FSAFE-6 | Watchdog / task overrun | `task_watchdog/`, `configCHECK_FOR_STACK_OVERFLOW` | Inject a blocked task | S |
| FSAFE-7 | Failsafe is visible to GCS and logger | `failsafe_flags`, `vehicle_status`, `logger` | Log review | S |

## 11. Flight and mission logging

| ID | Requirement | Implementation | Verification | Status |
|---|---|---|---|---|
| LOG-1 | Binary log with header and CRC | `logger/log_format.h`, `log_writer.c` | Host round-trip test (`test_log.c`) | I |
| LOG-2 | Log attitude, position, battery, mode, mission, failsafe | `logger/log_topics.c`, `config/log/logger.conf` | Inspect a SITL log | S |
| LOG-3 | Survive SD card slow/full | `logger/logger.c`, `log_storage.c` | Card removal test | S |
| LOG-4 | Post-flight conversion | `tools/log2csv.py` | Convert sample log | S |
| LOG-5 | Log start/stop events and drop counter | `log_status.msg` | Log review | S |

## 12. Cross-cutting requirements

| ID | Requirement | Implementation | Verification | Status |
|---|---|---|---|---|
| SYS-1 | Runs on FreeRTOS, OS layer named freertrtos | `os/freertrtos/`, `third_party/FreeRTOS-Kernel/` | Firmware build | S |
| SYS-2 | Fixed-priority task schedule | `src/main/task_registry.c`, `docs/task-schedule.md` | Timing trace | S |
| SYS-3 | Parameter storage | `src/middleware/params/`, `config/boards/thejas32/default.params` | Reboot persistence test | S |
| SYS-4 | Board portability | `src/hal/`, `boards/` | Build for SITL + target | S |
| SYS-5 | Testable without hardware | `tests/host/` | `make test` | I |
| SYS-6 | Operator documentation | `docs/` | Review | I |
