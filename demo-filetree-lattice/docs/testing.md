# Test strategy

The project uses a four-level test pyramid. Lower levels run on every commit;
higher levels run before a flight.

```
        Flight test        (rare, controlled, tethered first)
      HIL / bench          (hardware, props removed)
    SITL mission           (simulation, full stack)
  Host unit tests          (fast, every commit)
```

## 1. Host unit tests

Location: `tests/host/`. Run with:

```sh
make test
```

Current coverage (scaffold):

| Area | File | What it proves |
|---|---|---|
| Vector math | `test_math.c` | add/sub/dot/cross/normalize |
| Quaternions | `test_math.c` | identity, Euler round-trip, rotation |
| PID | `test_pid.c` | P, I, derivative, output clamp |
| Geodesy | `test_geo.c` | local NED ↔ global, distance, bearing |
| Mission | `test_mission.c` | add, validate, current, advance, invalid input |
| Attitude/rate control | `test_control.c` | quaternion sign, rate PID, saturation |
| Position control | `test_control.c` | hover thrust, horizontal response |
| Mixer and limits | `test_control.c` | quad-X signs, clamping, slew limit |
| Log format | `test_log.c` | header/record round-trip, CRC rejection |
| MAVLink framing | `test_mavlink.c` | v2 CRC, parser, command mapping, mission upload |
| Waypoints | `test_nav.c` | acceptance radius, altitude gate, geo conversion |

Add tests for every pure function before wiring it into a task. Use recorded
data (IMU, GNSS, RC) as golden files where possible.

## 2. SITL

Location: `tests/sitl/`. No plant model is checked in yet; the SITL build is a
planned target. The intended layout replaces the board HAL with a plant model
and a sensor simulator:

- `sim_plant.c` (planned): rigid-body quadrotor dynamics (6 DOF, simple drag).
- `sim_sensors.c` (planned): IMU/baro/mag/GNSS with noise and bias.
- The flight stack runs **unmodified** against the simulated sensors.
- MAVLink connects to QGC/Mission Planner over UDP.

Until those files exist, the scenarios below are a test plan, not executed
coverage.

SITL scenarios:

| Scenario | Expected result |
|---|---|
| Hover in stabilized | Altitude drift < 0.5 m/min, attitude stable |
| Position hold step | Settles within 1 m, no overshoot > 20% |
| 5-waypoint mission | All waypoints accepted, RTL at end |
| Mission abort | Hold then RTL |
| Battery low | RTL at threshold |
| RC loss | RTL, re-arm required |
| GCS loss | Mission continues, RTL after timeout |
| GNSS loss | Hold/alt-hold, safe landing |
| Wind gust | Recovers without saturation for > 2 s |

SITL is not a substitute for HIL: it does not exercise the real UART, SPI, PWM
or timing.

## 3. HIL / bench

Props **removed**, vehicle restrained:

- Flash the real firmware on the real board.
- Feed sensor data from a simulator or recorded replay.
- Measure PWM/DShot outputs with a logic analyser.
- Verify arming, mode changes, failsafe and logging.
- Pull the RC/telemetry cables and confirm the state machine.
- Run a 30-minute soak test and check for memory/stack leaks.

## 4. Flight test

Only after all lower levels pass. Use a checklist:

1. Verify firmware hash and parameter set.
2. Confirm failsafe actions and kill switch.
3. Calibrate accel/gyro/mag/baro.
4. Test motor order and direction with props removed.
5. Hover at 1–2 m in stabilized; check oscillations and drift.
6. Test altitude hold, then position hold.
7. Fly a short mission in a clear area.
8. Test RTL and one failsafe (e.g. RC loss) at safe altitude.
9. Inspect the log after every flight.

## 5. Fault injection

The following must be tested deliberately:

| Fault | Method |
|---|---|
| IMU stuck / spike | Replay a corrupted sensor stream |
| Baro drift | Pressure ramp in SITL |
| GNSS jump | Inject a position offset |
| RC loss | Turn off the transmitter |
| GCS loss | Unplug the telemetry radio |
| Low battery | Bench PSU ramp |
| SD card full/removed | Fill or remove the card |
| Task overrun | Breakpoint/block a task |
| Stack overflow | Reduce a stack in a test build |

## 6. Continuous integration

`.github/workflows/build.yml` runs the host tests on every push. Firmware
builds require a self-hosted runner with the RISC-V toolchain and the kernel
checkouts; they are optional until the port is available.

## 7. Definition of done for a module

- Interface documented in the header.
- Unit tests for pure logic.
- SITL scenario (if it affects flight).
- Failsafe behaviour defined.
- Parameters documented.
- Logged topics identified.
- No dynamic allocation in the periodic path.
- Stack and CPU budget measured.
