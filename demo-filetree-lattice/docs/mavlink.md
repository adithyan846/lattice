# MAVLink / GCS interface

The `mavlink` module is the only component that speaks to Mission Planner,
QGroundControl or any other MAVLink GCS. It never blocks the control loop.

## 1. Transports

| Transport | File | Typical use |
|---|---|---|
| UART telemetry radio | `src/drivers/telemetry/telemetry_uart.c` | SiK / 915 MHz / 433 MHz |
| USB CDC (future) | `mavlink_transport.c` | Bench configuration |
| UDP (SITL) | `boards/sitl/` | Software-in-the-loop |

`mavlink_transport` provides a byte-stream read/write interface. Framing,
CRC and message dispatch live in `mavlink.c` and `mavlink_receiver.c`.

## 2. Outgoing streams

Configured in `config/mavlink/streams.conf`:

| Message | Rate | Source topic | Purpose |
|---|---:|---|---|
| `HEARTBEAT` | 1 Hz | `vehicle_status` | Mode, armed state, system type |
| `SYS_STATUS` | 2 Hz | `battery_status`, `telemetry_status` | Battery, sensor health, link |
| `SYSTEM_TIME` | 1 Hz | `time` | GCS time sync |
| `GPS_RAW_INT` | 5 Hz | `sensor_gps` | Fix, satellites, HDOP |
| `GLOBAL_POSITION_INT` | 5 Hz | `vehicle_global_position` | Map position |
| `LOCAL_POSITION_NED` | 10 Hz | `vehicle_local_position` | Local position/velocity |
| `ATTITUDE` | 10 Hz | `vehicle_attitude` | Artificial horizon |
| `BATTERY_STATUS` | 1 Hz | `battery_status` | Voltage, current, remaining |
| `MISSION_CURRENT` | 2 Hz | `mission_current` | Mission progress |
| `STATUSTEXT` | event | `mavlink_log` | Warnings, arm reasons |

Streams are rate-limited by the `mavlink` task; a slow link drops old samples
rather than queuing them.

## 3. Incoming commands

`mavlink_commands.c` handles:

| MAVLink command | Action |
|---|---|
| `MAV_CMD_COMPONENT_ARM_DISARM` | Commander arm/disarm (with checks) |
| `MAV_CMD_DO_SET_MODE` | Mode change (mapped to internal mode) |
| `MAV_CMD_NAV_TAKEOFF` | Mission takeoff / guided takeoff |
| `MAV_CMD_NAV_LAND` | Land |
| `MAV_CMD_NAV_RETURN_TO_LAUNCH` | RTL |
| `MAV_CMD_DO_PAUSE_CONTINUE` | Pause/resume mission |
| `MAV_CMD_DO_SET_HOME` | Set home position |
| `PARAM_SET` | Parameter write |
| `MISSION_*` | Mission upload/download |

Every command is checked for:

1. target system/component (must match `MAV_SYS_ID` / `MAV_COMP_ID`),
2. authorization (arm/disarm may require GCS or switch),
3. current state (e.g. cannot start a mission while disarmed),
4. parameter range.

The result is sent as `COMMAND_ACK` with a MAVLink result code, and logged.

## 4. Mission protocol

```
GCS                          Vehicle
 │  MISSION_COUNT (N)          │
 │────────────────────────────►│  allocate/clear mission
 │  MISSION_REQUEST_INT (0)    │
 │◄────────────────────────────│
 │  MISSION_ITEM_INT (0)       │
 │────────────────────────────►│  validate item, store
 │  ...                        │
 │  MISSION_ACK                │
 │◄────────────────────────────│
 │  MISSION_SET_CURRENT        │
 │────────────────────────────►│  navigator cursor
```

- Items are stored in `lib/mission` (max 64) and persisted to
  `mission_storage` so a pre-loaded mission survives reboot.
- Validation happens per item and again before arming/starting.
- Upload is atomic: a new mission replaces the old one only after
  `MISSION_ACK(ACCEPTED)`.
- Download (`MISSION_REQUEST_LIST`) is supported so the GCS can verify the
  stored mission.

## 5. GCS display mapping

| GCS field | Source |
|---|---|
| Vehicle icon / map | `GLOBAL_POSITION_INT`, `GPS_RAW_INT` |
| Artificial horizon | `ATTITUDE` |
| Altitude / climb | `GLOBAL_POSITION_INT.relative_alt`, `LOCAL_POSITION_NED.vz` |
| Flight mode | `HEARTBEAT.custom_mode` |
| Armed | `HEARTBEAT.base_mode` safety-armed bit |
| Battery | `SYS_STATUS`, `BATTERY_STATUS` |
| Mission progress | `MISSION_CURRENT` |
| Warnings | `STATUSTEXT` |
| Link quality | `SYS_STATUS` / `RADIO_STATUS` (future) |

## 6. Parameters

`PARAM_REQUEST_LIST`, `PARAM_REQUEST_READ` and `PARAM_SET` are implemented over
the `params` middleware. Parameter names are ASCII, max 16 characters, and
stored with a CRC in flash. A failed flash write returns
`MAV_RESULT_FAILED` rather than silently pretending success.

## 7. Timeouts and link loss

- `MAV_GCS_TIMEOUT_MS` (default 5000) without a heartbeat sets
  `telemetry_status.connected = false`.
- Commander sees `failsafe_flags.gcs_loss` and applies the GCS failsafe policy.
- A recovered link clears the flag after two consecutive heartbeats.
- The vehicle never depends on the GCS for stabilization; loss of GCS is a
  mission-level event.

## 8. Testing

- `pymavlink` scripts in `tools/` connect to SITL over UDP.
- `mavlink_mission.py` uploads `config/missions/demo_mission.waypoints` and
  verifies `MISSION_ACK`.
- `mavlink_status.py` prints heartbeat, mode, battery and mission progress.
- A serial loopback test exercises framing/CRC without a radio.
