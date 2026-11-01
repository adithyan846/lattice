#!/usr/bin/env python3
"""Upload a QGC WPL 110 waypoint file to a freertrtos SITL/GCS link."""

from __future__ import annotations

import argparse
import time
from pathlib import Path

try:
    from pymavlink import mavutil
except ImportError as error:  # pragma: no cover - tooling only
    raise SystemExit("pymavlink is required: pip install pymavlink") from error

MAV_FRAME_GLOBAL_RELATIVE_ALT_INT = 6


def read_waypoints(path: Path):
    lines = [line for line in path.read_text().splitlines() if line.strip()]

    if not lines or not lines[0].startswith("QGC WPL"):
        raise ValueError("expected a QGC WPL 110 file")

    items = []

    for line in lines[1:]:
        fields = line.split()

        if len(fields) < 12:
            raise ValueError(f"malformed waypoint line: {line}")

        seq, current, frame, command = (int(fields[i]) for i in range(4))
        params = [float(value) for value in fields[4:11]]
        autocontinue = int(fields[11])
        items.append((seq, current, frame, command, params, autocontinue))

    return items


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("waypoints", type=Path)
    parser.add_argument("--connect", default="udp:127.0.0.1:14550")
    parser.add_argument("--timeout", type=float, default=30.0)
    args = parser.parse_args()

    items = read_waypoints(args.waypoints)
    master = mavutil.mavlink_connection(args.connect)
    print(f"waiting for heartbeat on {args.connect} ...")
    master.wait_heartbeat(timeout=args.timeout)
    print(f"connected to system {master.target_system} component {master.target_component}")

    master.mav.mission_count_send(
        master.target_system, master.target_component, len(items), 0
    )

    deadline = time.monotonic() + args.timeout

    while time.monotonic() < deadline:
        message = master.recv_match(
            type=["MISSION_REQUEST", "MISSION_REQUEST_INT", "MISSION_ACK"], blocking=True, timeout=2
        )

        if message is None:
            continue

        if message.get_type() in ("MISSION_REQUEST", "MISSION_REQUEST_INT"):
            seq = message.seq
            _, current, frame, command, params, autocontinue = items[seq]
            master.mav.mission_item_int_send(
                master.target_system,
                master.target_component,
                seq,
                MAV_FRAME_GLOBAL_RELATIVE_ALT_INT,
                command,
                current,
                autocontinue,
                params[0],
                params[1],
                params[2],
                params[3],
                int(params[4] * 1e7),
                int(params[5] * 1e7),
                params[6],
                0,
            )
            print(f"sent item {seq + 1}/{len(items)}")

        elif message.get_type() == "MISSION_ACK":
            if message.type == 0:
                print("mission accepted")
                return 0

            print(f"mission rejected: MAV_MISSION_RESULT={message.type}")
            return 1

    print("timed out waiting for the mission protocol to finish")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
