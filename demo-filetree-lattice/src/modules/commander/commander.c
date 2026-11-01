#include <modules/commander/commander.h>

#include <stddef.h>

#include <middleware/log/log.h>
#include <modules/commander/commander_state.h>
#include <modules/commander/home_position.h>

static commander_state_machine_t state;
static home_position_t home;

int commander_start(void)
{
    commander_state_init(&state);
    home_position_init(&home);

    /* TODO: create the 20 Hz commander task, subscribe to manual control,
     * battery, estimator, telemetry and command topics, run arming checks and
     * the failsafe state machine, and publish vehicle_status/actuator_armed. */
    FRT_LOG_WARN("commander: scaffold only, arming and failsafe not implemented");
    return -1;
}

int commander_stop(void)
{
    return 0;
}

int commander_arm(void)
{
    return commander_state_arm(&state);
}

int commander_disarm(void)
{
    return commander_state_disarm(&state);
}

int commander_set_mode(commander_nav_state_t mode)
{
    return commander_state_set_mode(&state, mode);
}

bool commander_is_armed(void)
{
    return state.arming == COMMANDER_ARMING_ARMED;
}

const char *commander_mode_name(commander_nav_state_t mode)
{
    switch (mode) {
    case COMMANDER_NAV_MANUAL:
        return "manual";

    case COMMANDER_NAV_STABILIZED:
        return "stabilized";

    case COMMANDER_NAV_ALT_HOLD:
        return "altitude hold";

    case COMMANDER_NAV_POS_HOLD:
        return "position hold";

    case COMMANDER_NAV_MISSION:
        return "mission";

    case COMMANDER_NAV_RTL:
        return "return to launch";

    case COMMANDER_NAV_LAND:
        return "land";

    default:
        return "unknown";
    }
}
