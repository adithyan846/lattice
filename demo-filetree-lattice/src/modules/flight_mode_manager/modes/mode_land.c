#include <modules/flight_mode_manager/modes/modes.h>

#include <stdbool.h>
#include <stddef.h>

#include <modules/flight_mode_manager/mode_utils.h>

#define LAND_DEFAULT_SPEED_MPS 0.5f
#define LAND_TOUCHDOWN_ALT_M   0.15f
#define LAND_HOVER_THRUST      0.35f

static float configured_descent_speed_mps = LAND_DEFAULT_SPEED_MPS;
static vec3_t land_target;
static float land_yaw_rad;
static bool land_initialized;

int mode_land_configure(float descent_speed_mps)
{
    if (descent_speed_mps <= 0.0f || descent_speed_mps > 2.0f) {
        return -1;
    }

    configured_descent_speed_mps = descent_speed_mps;
    return 0;
}

static int land_enter(mode_context_t *context)
{
    if (context == NULL) {
        return 0;
    }

    if (!context->position_valid && !context->altitude_valid) {
        return -1;
    }

    land_target = context->local_position_ned;
    land_yaw_rad = context->heading_rad;
    land_initialized = true;
    return 0;
}

static int land_update(mode_context_t *context, float dt_s)
{
    if (context == NULL || (!context->position_valid && !context->altitude_valid)) {
        return -1;
    }

    if (!land_initialized) {
        land_target = context->local_position_ned;
        land_yaw_rad = context->heading_rad;
        land_initialized = true;
    }

    /* NED: descending increases z. Stop the target at ground level; the
     * land_detector is responsible for confirming touchdown and disarming. */
    const bool descending = context->altitude_m > LAND_TOUCHDOWN_ALT_M;

    if (descending && dt_s > 0.0f) {
        land_target.z += configured_descent_speed_mps * dt_s;

        if (land_target.z > 0.0f) {
            land_target.z = 0.0f;
        }
    }

    mode_setpoint_t *setpoint = &context->output;

    setpoint->position_ned = land_target;
    setpoint->velocity_ned = vec3_make(0.0f, 0.0f,
                                      descending ? configured_descent_speed_mps : 0.0f);
    setpoint->yaw = land_yaw_rad;
    setpoint->yaw_rate = 0.0f;
    setpoint->thrust = LAND_HOVER_THRUST;
    setpoint->roll = context->roll;
    setpoint->pitch = context->pitch;

    return 0;
}

static void land_exit(mode_context_t *context)
{
    land_initialized = false;
    (void)context;
}

const flight_mode_ops_t mode_land_ops = {
    .id = FLIGHT_MODE_LAND,
    .name = "land",
    .enter = land_enter,
    .update = land_update,
    .exit = land_exit,
};
