#include <modules/flight_mode_manager/modes/modes.h>

#include <math.h>
#include <stdbool.h>
#include <stddef.h>

#include <lib/rtl/rtl.h>
#include <modules/flight_mode_manager/mode_utils.h>

#define RTL_DEFAULT_RETURN_ALT_M  30.0f
#define RTL_DEFAULT_DESCEND_ALT_M 5.0f
#define RTL_HOVER_THRUST          0.50f

static float configured_return_alt_m = RTL_DEFAULT_RETURN_ALT_M;
static float configured_descend_alt_m = RTL_DEFAULT_DESCEND_ALT_M;
static rtl_profile_t profile;

int mode_rtl_configure(float return_alt_m, float descend_alt_m)
{
    if (return_alt_m <= descend_alt_m || descend_alt_m < 0.0f) {
        return -1;
    }

    configured_return_alt_m = return_alt_m;
    configured_descend_alt_m = descend_alt_m;
    return 0;
}

static int rtl_mode_enter(mode_context_t *context)
{
    if (context == NULL) {
        return 0;
    }

    if (!context->home_valid) {
        return -1;
    }

    rtl_init(&profile, configured_return_alt_m, configured_descend_alt_m);
    return 0;
}

static int rtl_mode_update(mode_context_t *context, float dt_s)
{
    if (context == NULL || !context->home_valid || !context->position_valid) {
        return -1;
    }

    (void)dt_s;

    const rtl_phase_t phase =
        rtl_update(&profile, &context->local_position_ned, &context->home_position_ned, true);

    if (phase == RTL_PHASE_IDLE) {
        return -1;
    }

    vec3_t target = context->local_position_ned;
    float vertical_velocity_mps = 0.0f;

    switch (phase) {
    case RTL_PHASE_CLIMB:
        target.z = context->home_position_ned.z - profile.return_alt_m;
        vertical_velocity_mps = profile.cruise_speed_mps;
        break;

    case RTL_PHASE_CRUISE:
        target.x = context->home_position_ned.x;
        target.y = context->home_position_ned.y;
        target.z = context->home_position_ned.z - profile.return_alt_m;
        break;

    case RTL_PHASE_DESCEND:
        target.x = context->home_position_ned.x;
        target.y = context->home_position_ned.y;
        target.z = context->home_position_ned.z - profile.descend_alt_m;
        vertical_velocity_mps = profile.land_speed_mps;
        break;

    case RTL_PHASE_LAND:
        target = context->home_position_ned;
        vertical_velocity_mps = profile.land_speed_mps;
        break;

    case RTL_PHASE_DONE:
        target = context->home_position_ned;
        break;

    case RTL_PHASE_IDLE:
    default:
        return -1;
    }

    const float north = target.x - context->local_position_ned.x;
    const float east = target.y - context->local_position_ned.y;

    mode_setpoint_t *setpoint = &context->output;

    setpoint->position_ned = target;
    setpoint->velocity_ned = vec3_make(0.0f, 0.0f, vertical_velocity_mps);
    setpoint->thrust = RTL_HOVER_THRUST;

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

static void rtl_mode_exit(mode_context_t *context)
{
    (void)context;
}

const flight_mode_ops_t mode_rtl_ops = {
    .id = FLIGHT_MODE_RTL,
    .name = "rtl",
    .enter = rtl_mode_enter,
    .update = rtl_mode_update,
    .exit = rtl_mode_exit,
};
