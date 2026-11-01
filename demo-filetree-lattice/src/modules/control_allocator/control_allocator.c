#include <modules/control_allocator/control_allocator.h>

#include <stddef.h>

#include <lib/math/vec3.h>
#include <middleware/log/log.h>

void control_allocator_init(control_allocator_t *allocator)
{
    if (allocator == NULL) {
        return;
    }

    mixer_quad_x_default_config(&allocator->mixer);

    /* The ESC layer needs a defined ramp; the exact value depends on the
     * propulsion set. Zero disables slew limiting for now. */
    actuator_limits_init(&allocator->limits, MIXER_QUAD_X_MOTOR_COUNT, allocator->mixer.motor_min,
                         allocator->mixer.motor_max, 0.0f);
}

int control_allocator_step(control_allocator_t *allocator,
                           const vehicle_torque_setpoint_s *torque_setpoint,
                           const vehicle_thrust_setpoint_s *thrust_setpoint, float dt_s,
                           actuator_motors_s *motors)
{
    if (allocator == NULL || torque_setpoint == NULL || thrust_setpoint == NULL || motors == NULL) {
        return -1;
    }

    const vec3_t torque =
        vec3_make(torque_setpoint->xyz[0], torque_setpoint->xyz[1], torque_setpoint->xyz[2]);
    const float thrust = thrust_setpoint->xyz[2];

    if (mixer_quad_x_mix(&allocator->mixer, torque, thrust, motors->control) != 0) {
        return -1;
    }

    if (dt_s > 0.0f &&
        actuator_limits_apply(&allocator->limits, motors->control, MIXER_QUAD_X_MOTOR_COUNT, dt_s) !=
            0) {
        return -1;
    }

    motors->timestamp = thrust_setpoint->timestamp;
    return 0;
}

int control_allocator_start(void)
{
    /* TODO: subscribe to vehicle_torque_setpoint and vehicle_thrust_setpoint,
     * run allocation at the rate-loop frequency, apply arming state, and send
     * actuator_motors to the PWM/DShot output driver. */
    FRT_LOG_WARN("control_allocator: scaffold only, output task not implemented");
    return -1;
}

int control_allocator_stop(void)
{
    return 0;
}
