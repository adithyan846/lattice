# Safety

> This is a scaffold for a custom autopilot. It is not certified, not
> redundant, and not safe to fly as-is. A multicopter has spinning blades and
> can cause serious injury. Treat every powered test as hazardous.

## 1. Non-negotiable rules

1. **Props off** for all bench tests, firmware flashing and parameter changes.
2. A **hardware kill switch** that removes motor power must be independent of
   software and reachable by the operator.
3. **Arming requires explicit intent**: switch or GCS command, never a
   boot-time default.
4. **Failsafe must be tested** before every first flight of a new build.
5. **No code changes** between the tested binary and the flown binary.
6. **One variable at a time** during tuning.
7. **Log every flight** and review it before the next one.

## 2. Arming interlocks

The commander must refuse arming when any of these is true:

- estimator attitude invalid,
- gyro/accel calibration missing,
- battery below `BAT_ARM_MIN`,
- RC throttle not low (if RC is the arm source),
- an active failsafe,
- mission mode selected without a valid mission,
- safety switch not engaged (if fitted),
- board temperature/voltage out of range.

The refusal reason is shown on the GCS and written to the log.

## 3. Failsafe hierarchy

| Condition | Action |
|---|---|
| Battery low | RTL |
| Battery critical | Land now |
| RC loss | RTL |
| GCS loss | Continue mission, then RTL |
| GNSS loss | Hold altitude; RTL if home valid, else land |
| Estimator invalid | Stabilized/alt-hold fallback; land |
| Mission abort | Hold, then RTL |
| Task overrun | Failsafe or reboot |
| Kill switch | Motors off immediately |

Details: [failsafe.md](failsafe.md).

## 4. Operational limits

Set and enforce:

- maximum altitude (geofence),
- maximum distance from home (geofence),
- maximum horizontal/vertical speed,
- minimum battery reserve,
- maximum wind for the airframe,
- temperature range of the battery and electronics.

The scaffold has a placeholder for geofence in `lib/rtl/` and
`modules/navigator/`; implement it before autonomous flight.

## 5. Hardware recommendations

- Separate motor power and logic power; a brown-out must not reboot the flight
  controller with motors spinning.
- Use a power module with voltage/current sensing.
- Add a hardware arming switch and a hardware kill switch.
- Use a buzzer/LED for arming and failsafe state.
- Strain-relieve propellers, antenna and battery connectors.
- Verify the ESC protocol and motor order with props removed.

## 6. Software assurance

- No dynamic allocation in the control path.
- Fixed-size stacks and queues.
- Watchdog with a visible reset reason.
- CRC on parameters, missions and logs.
- Range checks on every external input (RC, MAVLink, GNSS, sensors).
- Never trust a single sensor for a safety-critical decision.
- Fail visibly: a warning must reach the GCS and the log.

## 7. Incident response

1. Kill motors (hardware switch first).
2. Secure the area; do not approach with props spinning.
3. Preserve the log and the flight controller state.
4. Record the firmware hash, parameters and environmental conditions.
5. Reproduce in SITL/HIL before changing code.
6. Add a regression test for the root cause.

## 8. Regulatory

Autonomous flight may require permits, a remote pilot, geofence approval and
insurance depending on your jurisdiction. This document is not legal advice.
