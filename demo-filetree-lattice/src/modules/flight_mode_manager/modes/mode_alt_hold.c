#include <modules/flight_mode_manager/modes/modes.h>

#include <stdbool.h>
#include <stddef.h>

#include <lib/math/pid.h>
#include <modules/flight_mode_manager/mode_utils.h>

/* Altitude hold feeds an altitude PID into collective thrust. The gains are
 * placeholders: they must be identified on the vehicle before flight. */
#define ALT_HOLD_MAX_TILT_RAD       0.610865f /* 35 degrees */
#define ALT_HOLD_MAX_YAW_RATE_RPS   1.5f
#define ALT_HOLD_MAX_CLIMB_MPS      1.0f
#define ALT_HOLD_THRUST_STICK_DZ    0.10f
#define ALT_HOLD_HOVER_THRUST       0.50f
#define ALT_HOLD_MIN_ALT_M          0.5f
#define ALT_HOLD_MAX_ALT_M          120.0f
#define ALT_HOLD_STICK_EXPO         0.2f

static pid_t altitude_pid;
static bool altitude_pid_initialized;
static float hold_altitude_m;

static int alt_hold_enter(mode_context_t *context)
{
    if (context == NULL) {
        return 0;
    }

    if (!context->altitude_valid) {
        return -1;
    }

    hold_altitude_m = context->altitude_m;

    if (!altitude_pid_initialized) {
        pid_init(&altitude_pid, 0.12f, 0.03f, 0.05f, -0.40f, 0.40f);
        pid_set_integrator_limits(&altitude_pid, -0.20f, 0.20f);
        pid_set_derivative_filter(&altitude_pid, 0.10f);
        altitude_pid_initialized = true;

    } else {
        pid_reset(&altitude_pid);
    }

    context->output.roll = 0.0f;
    context->output.pitch = 0.0f;
    context->output.yaw = context->heading_rad;
    return 0;
}

static int alt_hold_update(mode_context_t *context, float dt_s)
{
    if (context == NULL || !context->altitude_valid || !context->attitude_valid || dt_s <= 0.0f) {
        return -1;
    }

    /* The thrust stick commands a climb rate; integrating it into the hold
     * altitude gives a natural "release to hold" behaviour. */
    const float climb_mps =
        mode_deadband(context->thrust_stick, ALT_HOLD_THRUST_STICK_DZ) * ALT_HOLD_MAX_CLIMB_MPS;
    hold_altitude_m += climb_mps * dt_s;
    hold_altitude_m = mode_clamp(hold_altitude_m, ALT_HOLD_MIN_ALT_M, ALT_HOLD_MAX_ALT_M);

    const float thrust_correction = pid_update(&altitude_pid, hold_altitude_m, context->altitude_m, dt_s);

    mode_setpoint_t *setpoint = &context->output;

    setpoint->roll =
        mode_stick_to_angle(mode_expo(context->roll_stick, ALT_HOLD_STICK_EXPO), ALT_HOLD_MAX_TILT_RAD);
    setpoint->pitch =
        mode_stick_to_angle(mode_expo(context->pitch_stick, ALT_HOLD_STICK_EXPO), ALT_HOLD_MAX_TILT_RAD);
    setpoint->yaw = context->heading_rad;
    setpoint->yaw_rate =
        mode_expo(context->yaw_stick, ALT_HOLD_STICK_EXPO) * ALT_HOLD_MAX_YAW_RATE_RPS;
    setpoint->thrust = mode_clamp(ALT_HOLD_HOVER_THRUST + thrust_correction, 0.0f, 1.0f);

    setpoint->roll_rate = 0.0f;
    setpoint->pitch_rate = 0.0f;

    return 0;
}

static void alt_hold_exit(mode_context_t *context)
{
    (void)context;
}

const flight_mode_ops_t mode_alt_hold_ops = {
    .id = FLIGHT_MODE_ALT_HOLD,
    .name = "alt_hold",
    .enter = alt_hold_enter,
    .update = alt_hold_update,
    .exit = alt_hold_exit,
};
