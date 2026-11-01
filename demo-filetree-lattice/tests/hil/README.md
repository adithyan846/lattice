# Hardware-in-the-loop tests

HIL testing is required before any flight. The THEJAS32 image must be exercised
with the real sensors, ESCs and telemetry radio connected, while the position
and actuator outputs are validated against a simulator.

## Bench checklist

- [ ] Board boots, prints the banner and reports the correct git hash.
- [ ] IMU, barometer, magnetometer and GNSS are detected and healthy.
- [ ] RC channels map to the expected sticks and switches.
- [ ] ESC output is verified on a thrust stand with props removed.
- [ ] Motor order and direction match `config/airframes/quad_x/`.
- [ ] Telemetry links to Mission Planner / QGC at the configured baud.
- [ ] Arm/disarm works from RC, GCS and the shell.
- [ ] Low-battery and link-loss failsafes trigger the configured action.
- [ ] A log file is written, survives power loss, and decodes on the host.
- [ ] Watchdog resets the vehicle when a control task is suspended.

## Safety

- Props off for every bench test.
- Use a current-limited supply for the first ESC tests.
- Keep a hardware kill switch independent of the flight controller.
- Never disable a failsafe merely to make a test pass.
