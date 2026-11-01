# MAVLink

Optional upstream MAVLink C library. The scaffold implements the subset it
needs in `src/modules/mavlink/` (v2 framing, CRC extras, heartbeat, position,
attitude, battery, command and mission messages). If you enable
`FREERTRTOS_MAVLINK_DIR`, CMake adds that directory as a subproject.

GCS compatibility must be tested with both Mission Planner and QGroundControl;
do not assume a message layout from memory. Regenerate the CRC extra table from
the pinned `common.xml`.
