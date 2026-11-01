# Message definitions

`.msg` files are the source of truth for every uORB topic. The C structs are
generated into `generated/uorb_topics.h` by:

```sh
make msg
# or
python3 tools/msg_gen.py --input src/msg --output src/msg/generated
```

Every message must start with `uint64 timestamp`. Supported types:

```
uint8 uint16 uint32 uint64
int8  int16  int32  int64
float32 float64 bool char
type[N] field     # fixed-size array
```

The generated header is committed so a fresh checkout can compile without
running the generator first. Regenerate it whenever a `.msg` file changes.

## Topic families

| Family | Examples |
|---|---|
| Sensors | `sensor_imu`, `sensor_baro`, `sensor_mag`, `sensor_gps` |
| Estimation | `vehicle_attitude`, `vehicle_local_position`, `vehicle_global_position`, `estimator_status` |
| Control | `trajectory_setpoint`, `vehicle_attitude_setpoint`, `vehicle_rates_setpoint`, `vehicle_torque_setpoint`, `vehicle_thrust_setpoint`, `actuator_motors` |
| System | `vehicle_status`, `actuator_armed`, `failsafe_flags`, `battery_status`, `telemetry_status` |
| Mission | `mission_item`, `mission_current`, `home_position` |
| GCS / logging | `vehicle_command`, `vehicle_command_ack`, `mavlink_log`, `log_status` |

Do not add a field to the middle of an existing message: the logger and the
GCS mapping assume a stable layout. Append fields and bump the log version.
