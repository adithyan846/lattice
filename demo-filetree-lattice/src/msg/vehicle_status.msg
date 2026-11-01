# Aggregate vehicle state for GCS, logger and failsafe.
uint64 timestamp
uint8 arming_state
uint8 nav_state
bool failsafe
uint8 failsafe_reason
bool battery_low
bool rc_lost
bool gcs_lost
bool gnss_lost
bool mission_abort
