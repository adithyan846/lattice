#include <modules/flight_mode_manager/modes/modes.h>

#include <stddef.h>

#include <modules/flight_mode_manager/mode_utils.h>

/* Stabilized mode commands roll/pitch angles and a yaw rate. */
#define STABILIZED_MAX_TILT_RAD 0.610865f /* 35 degrees */
#define STABILIZED_MAX_YAW_RATE_RPS 1.5f
#define STABILIZED_STICK_EXPO 0.2f

static int stabilized_enter(mode_context_t *context)
{
    if (context == NULL) {
        return 0;
    }

    if (!context->attitude_valid) {
        return -1;
    }

    /* Start level and hold the current heading. */
    context->output.roll = 0.0f;
    context->output.pitch = 0.0f;
    context->output.yaw = context->heading_rad;
    context->output.yaw_rate = 0.0f;
    return 0;
}

static int stabilized_update(mode_context_t *context, float dt_s)
{
    if (context == NULL || !context->attitude_valid) {
        return -1;
    }

    (void)dt_s;

    mode_setpoint_t *setpoint = &context->output;

    setpoint->roll =
        mode_stick_to_angle(mode_expo(context->roll_stick, STABILIZED_STICK_EXPO), STABILIZED_MAX_TILT_RAD);
    setpoint->pitch = mode_stick_to_angle(mode_expo(context->pitch_stick, STABILIZED_STICK_EXPO),
                                          STABILIZED_MAX_TILT_RAD);
    setpoint->yaw = context->heading_rad;
    setpoint->yaw_rate =
        mode_expo(context->yaw_stick, STABILIZED_STICK_EXPO) * STABILIZED_MAX_YAW_RATE_RPS;
    setpoint->thrust = mode_clamp(0.5f * (context->thrust_stick + 1.0f), 0.0f, 1.0f);

    /* Rate fields are not used in stabilized mode. */
    setpoint->roll_rate = 0.0f;
    setpoint->pitch_rate = 0.0f;

    return 0;
}

static void stabilized_exit(mode_context_t *context)
{
    (void)context;
}

const flight_mode_ops_t mode_stabilized_ops = {
    .id = FLIGHT_MODE_STABILIZED,
    .name = "stabilized",
    .enter = stabilized_enter,
    .update = stabilized_update,
    .exit = stabilized_exit,
};
