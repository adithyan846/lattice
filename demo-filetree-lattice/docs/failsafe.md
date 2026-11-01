# Failsafe design

Failsafe is a **state machine owned by `commander`**. Detectors publish
evidence (`failsafe_flags`, `battery_status`, `telemetry_status`,
`estimator_status`); commander decides the action. No detector directly
commands motors.

## 1. Trigger → action matrix

| Trigger | Detection | Warning | Action after timeout | Recovery |
|---|---|---|---|---|
| Battery low | `battery_status.remaining` / voltage below `BAT_LOW_VOLTAGE` | `STATUSTEXT`, LED, GCS | RTL (or land if already low) | Replace battery; no auto-clear |
| Battery critical | Voltage below `BAT_CRIT_VOLTAGE` | Continuous warning | Land immediately | None in flight |
| RC loss | No valid RC frame for `COM_RC_LOSS_TIMEOUT_MS` | `failsafe_flags.rc_loss` | RTL, then land | RC returns → pilot must re-select mode |
| GCS loss | No MAVLink heartbeat for `COM_GCS_LOSS_TIMEOUT_MS` | `telemetry_status` | Continue mission, then RTL | Link returns → clears after 2 s |
| GNSS loss | Fix < 3D or no update for `COM_GNSS_LOSS_TIMEOUT_MS` | `failsafe_flags.gnss_loss` | Hold altitude, then RTL if home valid, else land | Fix returns → clear |
| Estimator invalid | `attitude_valid` false or position invalid in a position mode | `STATUSTEXT` | Fall back to stabilized/alt-hold, then land | Requires valid attitude |
| Mission abort | GCS `MAV_CMD_DO_SET_MODE` / mission command | Ack + status | Hold 2 s, then RTL | New mission/mode |
| Geofence breach | `lib/geofence` (future) | Warning | RTL/land | Clear breach |
| Task overrun | `task_watchdog` misses deadline | `failsafe_flags` | RTL/land or reboot | Reboot |
| Kill switch | Hardware input / `MAV_CMD_COMPONENT_ARM_DISARM` force | — | Disarm outputs immediately | Manual reset |

## 2. Action hierarchy

```
NONE < HOLD < RTL < LAND < DISARM
```

Commander always chooses the **most severe** active action. A lower-severity
trigger cannot cancel a higher-severity action. For example, GCS link recovery
does not cancel a battery-critical land.

## 3. Detection details

### Battery

- Voltage is filtered (moving average) before threshold comparison to avoid
  sag-induced nuisance triggers.
- `BAT_N_CELLS` converts pack voltage to per-cell voltage.
- Low threshold default 3.5 V/cell, critical 3.3 V/cell.
- Hysteresis: low clears only above 3.6 V/cell for 10 s. Critical never clears.

### RC

- `rc_update` publishes `rc_channels` with a valid flag and timestamp.
- A frame is valid only if the CRC/failsafe bit passes and channel values are
  in range.
- Loss is declared after `COM_RC_LOSS_TIMEOUT_MS` with no valid frame.

### GCS

- `mavlink` publishes `telemetry_status.connected` based on heartbeat age and
  link quality.
- GCS loss does **not** immediately RTL: the vehicle continues the mission for
  `COM_GCS_LOSS_TIMEOUT_MS`, then RTL. This avoids aborting a mission for a
  brief telemetry dropout.

### GNSS

- Requires at least a 3D fix, `satellites_used >= 6`, HDOP below threshold.
- Loss of GNSS in position hold triggers altitude hold + warning first; RTL
  only if the local estimate is still usable and home is valid.
- A GNSS jump (position innovation too large) is rejected by the estimator
  rather than followed.

### Mission abort

- GCS sends `MAV_CMD_DO_SET_MODE` with the RTL/land mode, or a mission
  `MAV_CMD_DO_PAUSE_CONTINUE` pause.
- `navigator` stops the mission cursor and publishes a hold setpoint.
- Commander applies the configured `COM_MISSION_ABORT_ACTION` (default RTL).

## 4. Failsafe state output

`failsafe_flags.msg` carries the individual triggers. `vehicle_status.msg`
carries the aggregate `failsafe` boolean and `failsafe_reason`. Both are:

- streamed to the GCS (`SYS_STATUS` / `STATUSTEXT`),
- written to the flight log,
- displayed on the on-board status LED (future).

## 5. Recovery rules

1. Failsafe actions latch until the trigger is clear **and** the pilot
   explicitly re-selects a mode.
2. A cleared trigger does not automatically resume the mission.
3. After an RTL failsafe, the vehicle lands and disarms; restart requires a new
   arm cycle.
4. Kill switch / forced disarm is always allowed and is not overridable by
   software.

## 6. Test matrix

| Test | Method | Pass criterion |
|---|---|---|
| Battery low | Bench PSU voltage ramp | RTL at threshold, no chatter |
| Battery critical | PSU below critical | Land, no RTL detour |
| RC loss | Power off transmitter | RTL after timeout, re-arm required |
| GCS loss | Unplug telemetry | Mission continues, RTL after timeout |
| GNSS loss | Shield antenna | Hold/alt-hold, safe RTL or land |
| Estimator invalid | Inject bad IMU | Attitude fallback, land |
| Mission abort | GCS abort | Hold then RTL |
| Watchdog | Block a task in debugger | Failsafe or reboot |
| Kill switch | Hardware switch | Motors stop immediately |

All tests are repeated in SITL and HIL before a flight test.
