#include <modules/mc_att_control/rate_controller.h>

#include <stddef.h>

#include <lib/math/pid.h>

static void configure_axis(pid_t *pid, float kp, float ki, float kd, float output_limit,
                           float integrator_limit, float derivative_filter_tau)
{
    pid_init(pid, kp, ki, kd, -output_limit, output_limit);
    pid_set_integrator_limits(pid, -integrator_limit, integrator_limit);
    pid_set_derivative_filter(pid, derivative_filter_tau);
}

void rate_controller_default_config(rate_controller_config_t *config)
{
    if (config == NULL) {
        return;
    }

    /* Placeholders. Identify the real gains on the vehicle before flight. */
    config->kp[0] = 0.15f;
    config->kp[1] = 0.15f;
    config->kp[2] = 0.20f;
    config->ki[0] = 0.02f;
    config->ki[1] = 0.02f;
    config->ki[2] = 0.03f;
    config->kd[0] = 0.002f;
    config->kd[1] = 0.002f;
    config->kd[2] = 0.0f;
    config->torque_limit = 1.0f;
    config->integrator_limit = 0.2f;
    config->derivative_filter_tau = 0.01f;
}

void rate_controller_init(rate_controller_t *controller, const rate_controller_config_t *config)
{
    if (controller == NULL) {
        return;
    }

    if (config != NULL) {
        controller->config = *config;

    } else {
        rate_controller_default_config(&controller->config);
    }

    for (int axis = 0; axis < 3; axis++) {
        configure_axis(&controller->axis[axis], controller->config.kp[axis],
                       controller->config.ki[axis], controller->config.kd[axis],
                       controller->config.torque_limit, controller->config.integrator_limit,
                       controller->config.derivative_filter_tau);
    }

    controller->initialized = true;
}

void rate_controller_reset(rate_controller_t *controller)
{
    if (controller == NULL) {
        return;
    }

    for (int axis = 0; axis < 3; axis++) {
        pid_reset(&controller->axis[axis]);
    }
}

int rate_controller_update(rate_controller_t *controller, vec3_t rate_setpoint,
                           vec3_t rate_measurement, float dt_s, vec3_t *torque)
{
    if (controller == NULL || torque == NULL || dt_s <= 0.0f) {
        return -1;
    }

    if (!controller->initialized) {
        rate_controller_init(controller, &controller->config);
    }

    const float setpoint[3] = {rate_setpoint.x, rate_setpoint.y, rate_setpoint.z};
    const float measurement[3] = {rate_measurement.x, rate_measurement.y, rate_measurement.z};
    float output[3];

    for (int axis = 0; axis < 3; axis++) {
        output[axis] = pid_update(&controller->axis[axis], setpoint[axis], measurement[axis], dt_s);

        if (output[axis] > controller->config.torque_limit) {
            output[axis] = controller->config.torque_limit;

        } else if (output[axis] < -controller->config.torque_limit) {
            output[axis] = -controller->config.torque_limit;
        }
    }

    *torque = vec3_make(output[0], output[1], output[2]);
    return 0;
}
