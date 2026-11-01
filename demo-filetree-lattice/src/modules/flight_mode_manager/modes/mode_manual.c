#include <modules/flight_mode_manager/modes/modes.h>

#include <stddef.h>

#include <modules/flight_mode_manager/mode_utils.h>

/* Manual mode maps sticks directly to body rates. The values below are
 * conservative placeholders; tune them on a thrust stand and in SITL first. */
#define MANUAL_MAX_ROLL_RATE_RPS  3.0f
#define MANUAL_MAX_PITCH_RATE_RPS 3.0f
#define MANUAL_MAX_YAW_RATE_RPS   1.5f
#define MANUAL_STICK_EXPO         0.2f

static int manual_enter(mode_context_t *context)
{
    if (context == NULL) {
        /* The mode manager calls enter() before it has a live context. */
        return 0;
    }

    if (!context->attitude_valid) {
        return -1;
    }

    context->output.roll = context->roll;
    context->output.pitch = context->pitch;
    context->output.yaw = context->heading_rad;
    return 0;
}

static int manual_update(mode_context_t *context, float dt_s)
{
    if (context == NULL) {
        return -1;
    }

    if (!context->attitude_valid) {
        return -1;
    }

    (void)dt_s;

    mode_setpoint_t *setpoint = &context->output;

    setpoint->roll_rate =
        mode_expo(context->roll_stick, MANUAL_STICK_EXPO) * MANUAL_MAX_ROLL_RATE_RPS;
    setpoint->pitch_rate =
        mode_expo(context->pitch_stick, MANUAL_STICK_EXPO) * MANUAL_MAX_PITCH_RATE_RPS;
    setpoint->yaw_rate =
        mode_expo(context->yaw_stick, MANUAL_STICK_EXPO) * MANUAL_MAX_YAW_RATE_RPS;

    /* Stick range [-1, 1] maps to normalised collective thrust [0, 1]. */
    setpoint->thrust = mode_clamp(0.5f * (context->thrust_stick + 1.0f), 0.0f, 1.0f);

    /* Keep the attitude fields coherent for downstream consumers that read
     * them even in rate mode. */
    setpoint->roll = context->roll;
    setpoint->pitch = context->pitch;
    setpoint->yaw = context->heading_rad;

    return 0;
}

static void manual_exit(mode_context_t *context)
{
    (void)context;
}

const flight_mode_ops_t mode_manual_ops = {
    .id = FLIGHT_MODE_MANUAL,
    .name = "manual",
    .enter = manual_enter,
    .update = manual_update,
    .exit = manual_exit,
};
