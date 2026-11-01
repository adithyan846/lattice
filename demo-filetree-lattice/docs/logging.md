# Flight logging

The logger records enough data to reconstruct a flight and a mission without
recording raw sensor streams at full rate.

## 1. Format

Binary, little-endian, inspired by ULog but intentionally simpler:

```
┌─────────────────────────────┐
│ Log header                  │
│  magic "FRTL"               │
│  version, start time        │
│  board id, git hash         │
│  parameter block length     │
├─────────────────────────────┤
│ Parameter block             │
│  [name, type, value] ...    │
├─────────────────────────────┤
│ Record                      │
│  magic 0xA5                 │
│  topic id (uint16)          │
│  timestamp (uint64)         │
│  payload length (uint16)    │
│  payload                    │
│  CRC32                      │
├─────────────────────────────┤
│ Record ...                  │
└─────────────────────────────┘
```

Definitions: `src/modules/logger/log_format.h`.

## 2. Logged topics

Configured in `config/log/logger.conf`:

| Topic | Rate | Why |
|---|---:|---|
| `vehicle_attitude` | 50 Hz | Attitude response, oscillations |
| `vehicle_local_position` | 20 Hz | Position/velocity tracking |
| `vehicle_global_position` | 5 Hz | Map reconstruction |
| `vehicle_status` | 5 Hz | Mode, arming, failsafe |
| `actuator_motors` | 50 Hz | Motor saturation, mixer issues |
| `battery_status` | 1 Hz | Power/endurance |
| `mission_current` | 2 Hz | Mission progress |
| `failsafe_flags` | 2 Hz | Why a failsafe fired |
| `estimator_status` | 2 Hz | Sensor health |
| `sensor_gps` | 5 Hz | Fix quality |
| `log_status` | 1 Hz | Drop counter |

The logger subscribes at these rates and writes records in timestamp order.
Raw IMU is **not** logged by default; enable `sensor_imu` for vibration
analysis, accepting a much larger file.

## 3. Buffering and storage

- A single-producer/single-consumer ring buffer decouples the logger task from
  the SD card.
- If the buffer is full, the oldest unsent record is dropped and
  `log_status.dropped_records` increments. Flight code is never blocked.
- The log file is pre-allocated when possible to reduce fragmentation.
- On a clean disarm, the logger flushes, writes the footer and closes the file.
- On power loss, the file remains readable up to the last complete record;
  `log2csv.py` stops at the first bad CRC.

File naming:

```
/fs/microsd/log/YYYY-MM-DD/HH-MM-SS_freertrtos.ftlog
```

## 4. Post-flight tools

```sh
python3 tools/log2csv.py flight.ftlog --out flight.csv
python3 tools/log2csv.py flight.ftlog --topic vehicle_attitude
python3 tools/log2csv.py flight.ftlog --summary
```

`--summary` prints duration, mode changes, max altitude, max speed, battery
minimum, failsafe events and mission completion.

## 5. Privacy and retention

- Logs contain GPS coordinates. Treat them as sensitive.
- Provide a `logger erase` command and document it.
- Keep at least the last 20 flights; rotate by free-space threshold.

## 6. Failure modes

| Failure | Behaviour |
|---|---|
| SD card absent | Logger disabled, warning at boot, flight allowed (configurable) |
| SD card full | Stop logging, keep flying, `log_status` warning |
| Write timeout | Drop records, mark log as truncated |
| Corrupt record | CRC rejects it; next record still parsed |
| Reboot mid-log | File readable up to last valid record |
