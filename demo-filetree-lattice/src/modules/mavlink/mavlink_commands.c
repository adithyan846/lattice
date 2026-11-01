#include <modules/mavlink/mavlink_commands.h>

#include <stddef.h>

uint8_t mavlink_command_map(const mavlink_command_long_t *command, mavlink_command_t *mapped)
{
    if (command == NULL || mapped == NULL) {
        return MAV_RESULT_FAILED;
    }

    mapped->action = MAVLINK_ACTION_NONE;
    mapped->altitude_m = 0.0f;
    mapped->latitude_deg = 0.0;
    mapped->longitude_deg = 0.0;
    mapped->set_home_from_current = false;

    switch (command->command) {
    case MAV_CMD_COMPONENT_ARM_DISARM:
        if (command->param1 < 0.5f) {
            mapped->action = MAVLINK_ACTION_DISARM;

        } else {
            mapped->action = MAVLINK_ACTION_ARM;
        }

        return MAV_RESULT_ACCEPTED;

    case MAV_CMD_DO_SET_MODE: {
        flight_mode_t mode;

        if (mavlink_flight_mode_from_custom_mode((uint8_t)command->param2, &mode) != 0) {
            return MAV_RESULT_DENIED;
        }

        mapped->action = MAVLINK_ACTION_SET_MODE;
        mapped->mode = mode;
        return MAV_RESULT_ACCEPTED;
    }

    case MAV_CMD_NAV_TAKEOFF:
        mapped->action = MAVLINK_ACTION_TAKEOFF;
        mapped->altitude_m = command->param7;
        return MAV_RESULT_ACCEPTED;

    case MAV_CMD_NAV_LAND:
        mapped->action = MAVLINK_ACTION_LAND;
        return MAV_RESULT_ACCEPTED;

    case MAV_CMD_NAV_RETURN_TO_LAUNCH:
        mapped->action = MAVLINK_ACTION_RTL;
        return MAV_RESULT_ACCEPTED;

    case MAV_CMD_DO_PAUSE_CONTINUE:
        mapped->action = command->param1 < 0.5f ? MAVLINK_ACTION_PAUSE_MISSION
                                                : MAVLINK_ACTION_RESUME_MISSION;
        return MAV_RESULT_ACCEPTED;

    case MAV_CMD_DO_SET_HOME:
        mapped->action = MAVLINK_ACTION_SET_HOME;

        if (command->param1 >= 0.5f) {
            mapped->set_home_from_current = true;

        } else {
            mapped->latitude_deg = command->param5;
            mapped->longitude_deg = command->param6;
            mapped->altitude_m = command->param7;
        }

        return MAV_RESULT_ACCEPTED;

    default:
        return MAV_RESULT_UNSUPPORTED;
    }
}

uint8_t mavlink_mode_from_flight_mode(flight_mode_t mode)
{
    if (mode < 0 || mode >= FLIGHT_MODE_COUNT) {
        return 0;
    }

    return (uint8_t)(mode + 1);
}

int mavlink_flight_mode_from_custom_mode(uint8_t custom_mode, flight_mode_t *mode)
{
    if (mode == NULL || custom_mode == 0 || custom_mode > FLIGHT_MODE_COUNT) {
        return -1;
    }

    *mode = (flight_mode_t)(custom_mode - 1);
    return 0;
}

const char *mavlink_action_name(mavlink_action_t action)
{
    switch (action) {
    case MAVLINK_ACTION_ARM:
        return "arm";

    case MAVLINK_ACTION_DISARM:
        return "disarm";

    case MAVLINK_ACTION_SET_MODE:
        return "set_mode";

    case MAVLINK_ACTION_TAKEOFF:
        return "takeoff";

    case MAVLINK_ACTION_LAND:
        return "land";

    case MAVLINK_ACTION_RTL:
        return "rtl";

    case MAVLINK_ACTION_PAUSE_MISSION:
        return "pause_mission";

    case MAVLINK_ACTION_RESUME_MISSION:
        return "resume_mission";

    case MAVLINK_ACTION_SET_HOME:
        return "set_home";

    case MAVLINK_ACTION_NONE:
    default:
        return "none";
    }
}
