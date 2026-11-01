#include <modules/flight_mode_manager/modes/modes.h>

#include <math.h>
#include <stddef.h>

/* Mission mode tracks the navigation target published by the navigator. The
 * navigator owns item sequencing and acceptance radius checks; this mode only
 * turns the target into a trajectory setpoint. */
#define MISSION_HOVER_THRUST 0.50f

static int mission_enter(mode_context_t *context)
{
    if (context == NULL) {
        return 0;
    }

    if (!context->mission_active || !context->target_valid) {
        return -1;
    }

    return 0;
}

static int mission_update(mode_context_t *context, float dt_s)
{
    if (context == NULL) {
        return -1;
    }

    if (!context->mission_active || !context->target_valid || !context->position_valid) {
        return -1;
    }

    (void)dt_s;

    mode_setpoint_t *setpoint = &context->output;
    const vec3_t target = context->target_position_ned;

    setpoint->position_ned = target;
    setpoint->velocity_ned = vec3_zero();
    setpoint->thrust = MISSION_HOVER_THRUST;

    /* Face the target while it is at least 0.5 m away, otherwise hold the
     * current heading. */
    const float north = target.x - context->local_position_ned.x;
    const float east = target.y - context->local_position_ned.y;

    if ((north * north + east * east) > 0.25f) {
        setpoint->yaw = atan2f(east, north);

    } else {
        setpoint->yaw = context->heading_rad;
    }

    setpoint->yaw_rate = 0.0f;
    setpoint->roll = context->roll;
    setpoint->pitch = context->pitch;

    return 0;
}

static void mission_exit(mode_context_t *context)
{
    (void)context;
}

const flight_mode_ops_t mode_mission_ops = {
    .id = FLIGHT_MODE_MISSION,
    .name = "mission",
    .enter = mission_enter,
    .update = mission_update,
    .exit = mission_exit,
};
