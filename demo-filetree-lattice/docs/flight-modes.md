# Flight modes and arming

## 1. Arming state machine

```
                 pre-arm checks pass
 DISARMED ───────────────────────────────► ARMED
    ▲                                        │
    │                                        │ disarm request
    │                                        │ landing detected
    │        failsafe / kill switch          │
    └────────────────────────────────────────┘
```

States are owned by `commander`. The rest of the stack only reads
`actuator_armed` and `vehicle_status`.

### Pre-arm checks (minimum)

| Check | Source | Parameter |
|---|---|---|
| Estimator attitude valid | `estimator_status` | — |
| Gyro bias converged | `sensors` | `COM_ARM_GYRO` |
| Accelerometer calibrated | `sensors` | `COM_ARM_ACCEL` |
| Magnetometer calibrated (if used) | `sensors` | `COM_ARM_MAG` |
| GNSS fix adequate for position modes | `sensor_gps` | `COM_ARM_GNSS` |
| Battery voltage above minimum | `battery_status` | `BAT_ARM_MIN` |
| RC present and throttle low | `manual_control_setpoint` | `COM_ARM_RC` |
| No active failsafe | `failsafe_flags` | — |
| Mission loaded (mission mode only) | `lib/mission` | — |
| Safety switch / GCS authorization | board input | `COM_ARM_AUTH` |

Arming is refused with a reason code that is sent as `STATUSTEXT` and logged.

## 2. Mode table

| Mode | ID | Pilot input | Altitude source | Horizontal source | Output stage |
|---|---:|---|---|---|---|
| Manual | 1 | Roll/pitch/yaw **rates**, thrust | none | none | `vehicle_rates_setpoint` |
| Stabilized | 2 | Roll/pitch **angle**, yaw rate, thrust | none | none | `vehicle_attitude_setpoint` |
| Altitude hold | 3 | Roll/pitch angle, yaw rate, climb rate | baro/GNSS | none | `vehicle_attitude_setpoint` + altitude loop |
| Position hold | 4 | Roll/pitch velocity, yaw rate, climb rate | baro/GNSS | GNSS/optical flow | `trajectory_setpoint` |
| Mission | 5 | None (abort only) | mission item | mission item | `trajectory_setpoint` |
| Return to home | 6 | None (abort only) | RTL profile | home position | `trajectory_setpoint` |
| Land | 7 | None | descent profile | current position | `trajectory_setpoint` |

Mode IDs are internal; MAVLink custom modes map to these in
`mavlink_commands.c`.

## 3. Mode behaviour

### Manual

- Sticks map directly to body rates and normalized thrust.
- No attitude or altitude assistance. This is the escape mode for a failed
  estimator, but it still requires a valid gyro.
- Expo and deadband are applied by `rc_update`, not by the controller.

### Stabilized

- Roll/pitch sticks command attitude angles (default ±35°).
- Yaw stick commands yaw rate.
- Thrust stick commands collective thrust.
- The attitude controller runs at 400 Hz; the rate loop at 1 kHz.
- This is the default assisted mode and the recommended first-flight mode.

### Altitude hold

- Roll/pitch behave as in stabilized.
- Thrust stick becomes a climb-rate command with a deadband that holds the
  current altitude.
- Altitude source: baro with GNSS blending when available.
- Mode entry captures the current altitude as the hold target.

### Position hold

- Roll/pitch sticks command horizontal velocity (default max 5 m/s).
- Yaw stick commands yaw rate; releasing it holds heading.
- Thrust stick commands climb rate; releasing it holds altitude.
- Requires a valid local position estimate. If position validity is lost, the
  mode falls back to altitude hold and warns the pilot.

### Mission

- `navigator_mission` walks the pre-loaded mission.
- Waypoints are accepted when within `NAV_ACC_RAD` (default 2 m) and the
  altitude error is within `NAV_ALT_ERR`.
- `NAV_LOITER_TIME` is honoured at each item.
- Commands: takeoff, waypoint, loiter, land, RTL, delay.
- GCS can pause/resume/abort; abort triggers the configured failsafe action.

### Return to home / launch

1. Climb to `RTL_RETURN_ALT` (or stay higher if already above).
2. Fly to the home position at `RTL_CRUISE_SPEED`.
3. Descend to `RTL_DESCEND_ALT`.
4. Land at home, or loiter if `RTL_LAND_ENABLE=0`.

Home is captured on arming (or on first good GNSS fix, configurable). If home
is invalid, RTL is rejected and the vehicle holds/lands instead.

### Land

- Descend at `MPC_LAND_SPEED` until `land_detector` confirms touchdown.
- Disarm after `COM_DISARM_LAND` seconds.
- Abortable by the pilot in manual/stabilized mode.

## 4. Transition rules

| From → To | Rule |
|---|---|
| Disarmed → any flight mode | Mode is stored but outputs stay inhibited |
| Manual ↔ Stabilized | Allowed while armed; attitude setpoint is initialised from current attitude |
| Stabilized → Altitude hold | Requires valid altitude estimate; capture current altitude |
| Altitude hold → Position hold | Requires valid local position; capture current position |
| Position hold → Mission/RTL | Requires valid mission/home; navigator takes ownership |
| Mission → RTL | GCS/pilot abort or failsafe |
| Any → Land | Failsafe or explicit command |
| Any → Disarm | Only on ground or kill switch |

A mode request that fails its prerequisites is **rejected**, not silently
substituted. The reason is published and shown on the GCS.

## 5. Failsafe interaction

Failsafe has priority over pilot mode requests except for an explicit kill
switch. The commander can force `RTL` or `LAND`; once the failsafe clears, the
pilot must explicitly re-select a mode. See [failsafe.md](failsafe.md).

## 6. Parameters

| Parameter | Default | Meaning |
|---|---:|---|
| `COM_ARM_AUTH` | 1 | Require GCS/switch authorization |
| `COM_ARM_GNSS` | 0 | Require GNSS fix before arming |
| `COM_DISARM_LAND` | 2 s | Auto-disarm delay after landing |
| `COM_RC_LOSS_TIMEOUT_MS` | 500 | RC loss detection |
| `COM_GCS_LOSS_TIMEOUT_MS` | 5000 | GCS loss detection |
| `NAV_ACC_RAD` | 2.0 m | Waypoint acceptance radius |
| `NAV_LOITER_TIME` | 0 s | Default loiter at each waypoint |
| `RTL_RETURN_ALT` | 30 m | RTL cruise altitude |
| `RTL_DESCEND_ALT` | 10 m | RTL descent altitude |
| `RTL_LAND_ENABLE` | 1 | Land at home after RTL |
| `MPC_LAND_SPEED` | 0.5 m/s | Land descent rate |
