#include <lib/math/pid.h>

#include <stddef.h>

static float clampf(float value, float min_value, float max_value)
{
    if (value < min_value) {
        return min_value;
    }

    if (value > max_value) {
        return max_value;
    }

    return value;
}

void pid_init(pid_t *pid, float kp, float ki, float kd, float output_min, float output_max)
{
    if (pid == NULL) {
        return;
    }

    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->integrator = 0.0f;
    pid->previous_measurement = 0.0f;
    pid->derivative = 0.0f;

    pid->output_min = output_min;
    pid->output_max = output_max;
    pid->integrator_min = output_min;
    pid->integrator_max = output_max;

    pid->derivative_filter_tau = 0.0f;
    pid->initialized = false;
}

void pid_reset(pid_t *pid)
{
    if (pid == NULL) {
        return;
    }

    pid->integrator = 0.0f;
    pid->previous_measurement = 0.0f;
    pid->derivative = 0.0f;
    pid->initialized = false;
}

void pid_set_integrator_limits(pid_t *pid, float min_value, float max_value)
{
    if (pid == NULL) {
        return;
    }

    pid->integrator_min = min_value;
    pid->integrator_max = max_value;
    pid->integrator = clampf(pid->integrator, min_value, max_value);
}

void pid_set_derivative_filter(pid_t *pid, float tau_s)
{
    if (pid == NULL) {
        return;
    }

    pid->derivative_filter_tau = tau_s > 0.0f ? tau_s : 0.0f;
}

float pid_update(pid_t *pid, float setpoint, float measurement, float dt)
{
    if (pid == NULL || dt <= 0.0f) {
        return 0.0f;
    }

    if (!pid->initialized) {
        pid->previous_measurement = measurement;
        pid->initialized = true;
    }

    const float error = setpoint - measurement;
    const float proportional = pid->kp * error;

    /* Integrate first, then clamp. The clamp prevents wind-up when the
     * actuator is saturated for a long time. */
    pid->integrator += pid->ki * error * dt;
    pid->integrator = clampf(pid->integrator, pid->integrator_min, pid->integrator_max);

    /* Derivative on measurement avoids a kick when the setpoint steps. */
    const float raw_derivative = -(measurement - pid->previous_measurement) / dt;

    if (pid->derivative_filter_tau > 0.0f) {
        const float alpha = dt / (pid->derivative_filter_tau + dt);
        pid->derivative += alpha * (raw_derivative - pid->derivative);
    } else {
        pid->derivative = raw_derivative;
    }

    pid->previous_measurement = measurement;

    const float output = proportional + pid->integrator + pid->kd * pid->derivative;
    return clampf(output, pid->output_min, pid->output_max);
}
