#!/usr/bin/env python3
"""Decode freertrtos .ftlog files into CSV and a flight summary.

The scaffold log format is intentionally simple; topic payloads are the packed
uORB structs from src/msg/generated/uorb_topics.h. Schemas for the default
topics are listed below. Unknown topics are written as hex.
"""

from __future__ import annotations

import argparse
import csv
import struct
import sys
from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path

HEADER_MAGIC = b"FRTL"
HEADER_SIZE = 26
RECORD_MAGIC = 0xA5
FORMAT_VERSION = 1

# Matches log_topics_default() in src/modules/logger/log_topics.c.
TOPIC_NAMES = {
    1: "vehicle_attitude",
    2: "vehicle_local_position",
    3: "vehicle_global_position",
    4: "vehicle_status",
    5: "actuator_motors",
    6: "battery_status",
    7: "mission_current",
    8: "failsafe_flags",
    9: "estimator_status",
    10: "sensor_gps",
}

# struct format, field names. All payloads are little-endian and packed.
SCHEMAS: dict[str, tuple[str, list[str]]] = {
    "vehicle_attitude": ("<Q4f3f", ["timestamp", "q0", "q1", "q2", "q3", "roll", "pitch", "yaw"]),
    "vehicle_local_position": (
        "<Q7f4B",
        ["timestamp", "x", "y", "z", "vx", "vy", "vz", "heading", "xy_valid", "z_valid",
         "v_xy_valid", "v_z_valid"],
    ),
    "vehicle_status": (
        "<Q4B5B",
        ["timestamp", "arming_state", "nav_state", "failsafe", "failsafe_reason",
         "battery_low", "rc_lost", "gcs_lost", "gnss_lost", "mission_abort"],
    ),
    "battery_status": (
        "<Q4f3B",
        ["timestamp", "voltage_v", "current_a", "discharged_mah", "remaining",
         "cell_count", "low_voltage", "critical_voltage"],
    ),
    "mission_current": ("<QHHB", ["timestamp", "seq", "total", "valid"]),
}


@dataclass
class LogHeader:
    version: int
    start_time_us: int
    board_id: int
    git_hash: int
    parameter_block_length: int


@dataclass
class Record:
    topic_id: int
    timestamp: int
    payload: bytes


def decode_header(data: bytes) -> LogHeader:
    if len(data) < HEADER_SIZE or data[:4] != HEADER_MAGIC:
        raise ValueError("not a freertrtos log (bad magic)")

    version, start_time_us, board_id, git_hash, parameter_length = struct.unpack_from(
        "<HQIII", data, 4
    )

    if version != FORMAT_VERSION:
        raise ValueError(f"unsupported log version {version}")

    return LogHeader(version, start_time_us, board_id, git_hash, parameter_length)


def iter_records(data: bytes, offset: int):
    while offset + 17 <= len(data):
        if data[offset] != RECORD_MAGIC:
            offset += 1
            continue

        topic_id = struct.unpack_from("<H", data, offset + 1)[0]
        timestamp = struct.unpack_from("<Q", data, offset + 3)[0]
        length = struct.unpack_from("<H", data, offset + 11)[0]

        if length > 256 or offset + 13 + length + 4 > len(data):
            return

        payload = data[offset + 13 : offset + 13 + length]
        crc = struct.unpack_from("<I", data, offset + 13 + length)[0]

        import zlib

        if zlib.crc32(data[offset : offset + 13 + length]) & 0xFFFFFFFF != crc:
            return

        yield Record(topic_id, timestamp, payload)
        offset += 13 + length + 4


def decode_payload(topic: str, payload: bytes):
    schema = SCHEMAS.get(topic)

    if schema is None:
        return {"hex": payload.hex()}

    fmt, names = schema

    if struct.calcsize(fmt) > len(payload):
        return {"hex": payload.hex()}

    values = struct.unpack_from(fmt, payload)
    return dict(zip(names, values))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log", type=Path, help="input .ftlog file")
    parser.add_argument("--out", type=Path, help="output CSV (single topic only)")
    parser.add_argument("--topic", help="only decode one topic")
    parser.add_argument("--summary", action="store_true", help="print a flight summary")
    args = parser.parse_args()

    data = args.log.read_bytes()

    try:
        header = decode_header(data)
    except ValueError as error:
        print(f"error: {error}", file=sys.stderr)
        return 1

    records = list(iter_records(data, HEADER_SIZE + header.parameter_block_length))
    by_topic: dict[str, list[dict]] = defaultdict(list)

    for record in records:
        name = TOPIC_NAMES.get(record.topic_id, f"topic_{record.topic_id}")

        if args.topic and name != args.topic:
            continue

        row = decode_payload(name, record.payload)
        row.setdefault("time_s", record.timestamp / 1e6)
        by_topic[name].append(row)

    if args.topic:
        rows = by_topic.get(args.topic, [])
        output = args.out or Path(f"{args.log.stem}_{args.topic}.csv")

        with output.open("w", newline="") as handle:
            if rows:
                writer = csv.DictWriter(handle, fieldnames=list(rows[0].keys()))
                writer.writeheader()
                writer.writerows(rows)

        print(f"wrote {len(rows)} records to {output}")
        return 0

    if args.summary or not args.out:
        print(f"log: {args.log}")
        print(f"start: {header.start_time_us / 1e6:.3f} s")
        print(f"records: {len(records)}")

        for name, rows in sorted(by_topic.items()):
            print(f"  {name:24s} {len(rows):8d}")

        positions = by_topic.get("vehicle_local_position", [])

        if positions:
            max_alt = max(-row["z"] for row in positions)
            max_speed = max(
                (row["vx"] ** 2 + row["vy"] ** 2 + row["vz"] ** 2) ** 0.5 for row in positions
            )
            print(f"max altitude: {max_alt:.1f} m")
            print(f"max speed:    {max_speed:.1f} m/s")

        batteries = by_topic.get("battery_status", [])

        if batteries:
            print(f"min battery:  {min(row['remaining'] for row in batteries) * 100.0:.0f} %")

    if args.out:
        base = args.out.with_suffix("")

        for name, rows in by_topic.items():
            output = Path(f"{base}_{name}.csv")

            with output.open("w", newline="") as handle:
                writer = csv.DictWriter(handle, fieldnames=list(rows[0].keys()))
                writer.writeheader()
                writer.writerows(rows)

            print(f"wrote {len(rows)} {name} records to {output}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
