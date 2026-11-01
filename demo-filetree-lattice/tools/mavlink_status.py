#!/usr/bin/env python3
"""Print live freertrtos status received over MAVLink."""

from __future__ import annotations

import argparse
import time

try:
    from pymavlink import mavutil
except ImportError as error:  # pragma: no cover
    raise SystemExit("pymavlink is required: pip install pymavlink") from error

MODES = {
    1: "manual",
    2: "stabilized",
    3: "alt_hold",
    4: "pos_hold",
    5: "mission",
    6: "rtl",
    7: "land",
}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--connect", default="udp:127.0.0.1:14550")
    parser.add_argument("--rate", type=float, default=1.0, help="print interval in seconds")
    args = parser.parse_args()

    master = mavutil.mavlink_connection(args.connect)
    print(f"waiting for heartbeat on {args.connect} ...")
    master.wait_heartbeat(timeout=30)

    last_print = 0.0
    position = None
    battery = None
    mission = None

    while True:
        message = master.recv_match(
            type=["HEARTBEAT", "GLOBAL_POSITION_INT", "BATTERY_STATUS", "MISSION_CURRENT"],
            blocking=True,
            timeout=2,
        )

        if message is None:
            continue

        kind = message.get_type()

        if kind == "GLOBAL_POSITION_INT":
            position = message
        elif kind == "BATTERY_STATUS":
            battery = message
        elif kind == "MISSION_CURRENT":
            mission = message
        elif kind == "HEARTBEAT":
            now = time.monotonic()

            if now - last_print < args.rate:
                continue

            last_print = now
            armed = bool(message.base_mode & mavutil.mavlink.MAV_MODE_FLAG_SAFETY_ARMED)
            mode = MODES.get(message.custom_mode, f"custom {message.custom_mode}")
            print(f"mode={mode:12s} armed={'yes' if armed else 'no ':3s}", end="")

            if position is not None:
                print(
                    f" lat={position.lat / 1e7:.7f} lon={position.lon / 1e7:.7f}"
                    f" alt={position.relative_alt / 1000.0:6.1f} m",
                    end="",
                )

            if battery is not None:
                voltage = battery.voltages[0] / 1000.0 if battery.voltages else 0.0
                print(f" batt={voltage:5.2f} V {battery.battery_remaining:3d}%", end="")

            if mission is not None:
                print(f" mission={mission.seq}", end="")

            print()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
