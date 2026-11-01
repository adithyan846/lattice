# Implementation roadmap

This scaffold is deliberately broad. Build it in vertical slices so each stage
is testable before the next one depends on it.

## Phase 0 — Board and OS bring-up

- [ ] Flash a blinky/console image.
- [ ] Verify clock, SysTick, UART shell and GPIO.
- [ ] Measure context-switch latency and tick cost.
- [ ] Implement `frt_task_create`, queues, mutexes and time wrappers.
- [ ] Verify stack overflow and assert hooks.

Exit: `status` command prints uptime and task list over UART.

## Phase 1 — Sensor path

- [ ] IMU driver + SPI HAL, publish `sensor_imu` at 1 kHz.
- [ ] Baro + mag drivers.
- [ ] Sensor calibration and timestamping.
- [ ] `sensors` task with health flags.
- [ ] Log raw IMU for vibration analysis.

Exit: stationary gyro < 0.5 °/s, accel ≈ 1 g, baro/mag sane.

## Phase 2 — Attitude estimation and control

- [ ] Attitude estimator (Mahony/complementary first, EKF later).
- [ ] `mc_att_control` quaternion + rate PID.
- [ ] `control_allocator` quad-X mixer with limits.
- [ ] PWM/DShot output, arming gate.
- [ ] Stabilized mode on a test stand.

Exit: attitude tracking on a rate table or tethered hover.

## Phase 3 — Position and altitude

- [ ] Baro altitude + GNSS local position.
- [ ] `mc_pos_control` position/velocity loops.
- [ ] Altitude hold and position hold modes.
- [ ] Land detector and auto-disarm.

Exit: stable hover, altitude hold ±0.5 m, position hold ±1 m.

## Phase 4 — Mission and RTL

- [ ] `lib/mission` storage + validation (host-tested).
- [ ] `navigator_mission` waypoint sequencing.
- [ ] `navigator_rtl` climb/cruise/descend.
- [ ] Home position capture.
- [ ] Mission mode and RTL mode.

Exit: 5-waypoint SITL mission, then a real mission in a clear area.

## Phase 5 — GCS and failsafe

- [ ] MAVLink transport, heartbeat, streams.
- [ ] Command handling (arm, mode, RTL, land).
- [ ] Mission upload/download.
- [ ] Parameter protocol.
- [ ] Failsafe matrix + GCS display.

Exit: full mission flown from Mission Planner/QGC, failsafes tested.

## Phase 6 — Logging and hardening

- [ ] Binary logger + SD storage + `log2csv.py`.
- [ ] Log all flight/mission/failsafe topics.
- [ ] Watchdog and fault injection.
- [ ] Long soak test, stack/CPU measurements.
- [ ] Geofence and data-link quality.

Exit: post-flight log reconstructs the entire mission.

## Phase 7 — Beyond the brief

- [ ] EKF with GNSS/baro/mag fusion and innovation checks.
- [ ] Optical flow / rangefinder for indoor flight.
- [ ] Terrain following.
- [ ] Companion computer / MAVSDK.
- [ ] Redundant IMU and voting.
- [ ] Bootloader, firmware update, secure boot.

## Suggested first commit sequence

1. `feat(os): add freertrtos task/queue/time wrappers`
2. `feat(hal): add UART/SPI/GPIO interfaces and THEJAS32 BSP`
3. `feat(drivers): add IMU/baro/mag drivers`
4. `feat(estimator): add attitude estimator`
5. `feat(control): add attitude/rate control and quad-X mixer`
6. `feat(nav): add mission storage and waypoint navigator`
7. `feat(mavlink): add heartbeat and command handling`
8. `feat(logger): add binary flight log`
9. `test: add host and SITL coverage`

Keep each commit buildable and testable.
