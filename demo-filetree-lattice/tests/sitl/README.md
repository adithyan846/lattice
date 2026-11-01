# SITL tests

The SITL target uses `boards/sitl` and the POSIX port of FreeRTOS. In this
scaffold the flight modules return `-1` from their `*_start()` functions, so
there is no flyable SITL image yet. The intended test plan is:

1. Build the POSIX target:

   ```sh
   cmake -S . -B build/sitl -DFREERTRTOS_BOARD=sitl \
       -DFREERTRTOS_BUILD_FIRMWARE=ON -DFREERTRTOS_BUILD_TESTS=OFF
   cmake --build build/sitl
   ```

2. Start the simulator and wait for `HEARTBEAT`:

   ```sh
   python3 tools/mavlink_status.py --connect udp:127.0.0.1:14550
   ```

3. Upload the demo mission:

   ```sh
   python3 tools/mavlink_mission.py config/missions/demo_mission.waypoints
   ```

4. Verify arming, mode changes, RTL and failsafe behaviour. Record the log and
   decode it with `tools/log2csv.py --summary`.

Until the module stubs are implemented, only the host unit tests are expected
to pass. Do not treat an empty SITL run as evidence of flight readiness.
