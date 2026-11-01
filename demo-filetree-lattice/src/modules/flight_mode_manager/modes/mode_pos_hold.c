#include <modules/flight_mode_manager/modes/modes.h>

#include <math.h>
#include <stddef.h>

#include <modules/flight_mode_manager/mode_utils.h>

/* Position hold turns sticks into velocity commands and integrates a position
 * target that mc_pos_control can track. */
#define POS_HOLD_MAX_VELOCITY_MPS  5.0f
#define POS_HOLD_MAX_CLIMB_MPS     1.0f
#define POS_HOLD_MAX_YAW_RATE_RPS  1.5f
#define POS_HOLD_THRUST_STICK_DZ   0.10f
#define POS_HOLD_HOVER_THRUST      0.50f
#define POS_HOLD_MIN_ALT_M         0.5f
#define POS_HOLD_MAX_ALT_M         120.0f
#define POS_HOLD_STICK_EXPO        0.2f
#define POS_HOLD_PI                3.14159265f

static vec3_t hold_position;
static float hold_yaw_rad;
static bool initialized;

static int pos_hold_enter(mode_context_t *context)
{
    if (context == NULL) {
        return 0;
    }

    if (!context->position_valid) {
        return -1;
    }

    hold_position = context->local_position_ned;
    hold_yaw_rad = context->heading_rad;
    initialized = true;
    return 0;
}

static int pos_hold_update(mode_context_t *context, float dt_s)
{
    if (context == NULL || !context->position_valid || !context->attitude_valid || dt_s <= 0.0f) {
        return -1;
    }

    if (!initialized) {
        hold_position = context->local_position_ned;
        hold_yaw_rad = context->heading_rad;
        initialized = true;
    }

    const float velocity_north =
        mode_expo(context->roll_stick, POS_HOLD_STICK_EXPO) * POS_HOLD_MAX_VELOCITY_MPS;
    const float velocity_east =
        mode_expo(context->pitch_stick, POS_HOLD_STICK_EXPO) * POS_HOLD_MAX_VELOCITY_MPS;
    const float climb_mps =
        mode_deadband(context->thrust_stick, POS_HOLD_THRUST_STICK_DZ) * POS_HOLD_MAX_CLIMB_MPS;

    /* NED: z grows downwards, so a positive climb decreases z. */
    hold_position.x += velocity_north * dt_s;
    hold_position.y += velocity_east * dt_s;
    hold_position.z -= climb_mps * dt_s;
    hold_position.z = mode_clamp(hold_position.z, -POS_HOLD_MAX_ALT_M, -POS_HOLD_MIN_ALT_M);

    hold_yaw_rad +=
        mode_expo(context->yaw_stick, POS_HOLD_STICK_EXPO) * POS_HOLD_MAX_YAW_RATE_RPS * dt_s;

    /* Keep yaw wrapped to [-pi, pi]. */
    while (hold_yaw_rad > POS_HOLD_PI) {
        hold_yaw_rad -= 2.0f * POS_HOLD_PI;
    }

    while (hold_yaw_rad < -POS_HOLD_PI) {
        hold_yaw_rad += 2.0f * POS_HOLD_PI;
    }

    mode_setpoint_t *setpoint = &context->output;

    setpoint->position_ned = hold_position;
    setpoint->velocity_ned = vec3_make(velocity_north, velocity_east, -climb_mps);
    setpoint->yaw = hold_yaw_rad;
    setpoint->yaw_rate = 0.0f;
    setpoint->thrust = POS_HOLD_HOVER_THRUST;

    /* mc_pos_control owns roll/pitch; keep current attitude for consumers that
     * read it before the position loop runs. */
    setpoint->roll = context->roll;
    setpoint->pitch = context->pitch;

    return 0;
}

static void pos_hold_exit(mode_context_t *context)
{
    (void)context;
}

const flight_mode_ops_t mode_pos_hold_ops = {
    .id = FLIGHT_MODE_POS_HOLD,
    .name = "pos_hold",
    .enter = pos_hold_enter,
    .update = pos_hold_update,
    .exit = pos_hold_exit,
};
