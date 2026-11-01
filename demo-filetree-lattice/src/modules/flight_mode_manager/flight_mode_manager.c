#include <modules/flight_mode_manager/flight_mode_manager.h>

#include <stddef.h>

#include <middleware/log/log.h>
#include <modules/flight_mode_manager/modes/modes.h>

static const flight_mode_ops_t *active_mode;
static flight_mode_t requested_mode = FLIGHT_MODE_STABILIZED;

const flight_mode_ops_t *flight_mode_get_ops(flight_mode_t mode)
{
    switch (mode) {
    case FLIGHT_MODE_MANUAL:
        return &mode_manual_ops;

    case FLIGHT_MODE_STABILIZED:
        return &mode_stabilized_ops;

    case FLIGHT_MODE_ALT_HOLD:
        return &mode_alt_hold_ops;

    case FLIGHT_MODE_POS_HOLD:
        return &mode_pos_hold_ops;

    case FLIGHT_MODE_MISSION:
        return &mode_mission_ops;

    case FLIGHT_MODE_RTL:
        return &mode_rtl_ops;

    case FLIGHT_MODE_LAND:
        return &mode_land_ops;

    default:
        return NULL;
    }
}

int flight_mode_manager_start(void)
{
    /* TODO: create the 100 Hz mode task, subscribe to estimator, manual
     * control, mission and failsafe topics, and call the active mode's
     * update() to produce trajectory/attitude/rate setpoints. */
    active_mode = flight_mode_get_ops(requested_mode);
    FRT_LOG_WARN("flight_mode_manager: scaffold only, mode logic not implemented");
    return -1;
}

int flight_mode_manager_stop(void)
{
    return 0;
}

int flight_mode_manager_set_mode(flight_mode_t mode)
{
    const flight_mode_ops_t *ops = flight_mode_get_ops(mode);

    if (ops == NULL) {
        return -1;
    }

    if (active_mode != NULL && active_mode->exit != NULL) {
        active_mode->exit(NULL);
    }

    active_mode = ops;
    requested_mode = mode;

    if (active_mode->enter != NULL && active_mode->enter(NULL) != 0) {
        return -1;
    }

    return 0;
}

flight_mode_t flight_mode_manager_get_mode(void)
{
    return requested_mode;
}
