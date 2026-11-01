#ifndef MODULES_MAVLINK_MAVLINK_COMMANDS_H
#define MODULES_MAVLINK_MAVLINK_COMMANDS_H

#include <stdbool.h>
#include <stdint.h>

#include <modules/flight_mode_manager/mode.h>
#include <modules/mavlink/mavlink_messages.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MAVLINK_ACTION_NONE = 0,
    MAVLINK_ACTION_ARM,
    MAVLINK_ACTION_DISARM,
    MAVLINK_ACTION_SET_MODE,
    MAVLINK_ACTION_TAKEOFF,
    MAVLINK_ACTION_LAND,
    MAVLINK_ACTION_RTL,
    MAVLINK_ACTION_PAUSE_MISSION,
    MAVLINK_ACTION_RESUME_MISSION,
    MAVLINK_ACTION_SET_HOME,
} mavlink_action_t;

typedef struct {
    mavlink_action_t action;
    flight_mode_t mode;
    double latitude_deg;
    double longitude_deg;
    float altitude_m;
    bool set_home_from_current;
} mavlink_command_t;

/**
 * Map a decoded COMMAND_LONG to an internal action. Returns 0 on success and
 * MAV_RESULT_UNSUPPORTED for unknown commands.
 */
uint8_t mavlink_command_map(const mavlink_command_long_t *command, mavlink_command_t *mapped);

/**
 * Project-specific custom mode mapping used in HEARTBEAT and DO_SET_MODE.
 * Custom mode = flight_mode_t + 1, so 1=manual, 2=stabilized, ... 7=land.
 */
uint8_t mavlink_mode_from_flight_mode(flight_mode_t mode);
int mavlink_flight_mode_from_custom_mode(uint8_t custom_mode, flight_mode_t *mode);

const char *mavlink_action_name(mavlink_action_t action);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_MAVLINK_MAVLINK_COMMANDS_H */
