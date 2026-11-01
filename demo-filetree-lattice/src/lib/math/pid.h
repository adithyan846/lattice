#ifndef LIB_MATH_PID_H
#define LIB_MATH_PID_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * PID controller with derivative-on-measurement and clamped integrator.
 *
 * Units are whatever the caller uses consistently: error units per output
 * unit. `dt` is in seconds.
 */
typedef struct {
    float kp;
    float ki;
    float kd;

    float integrator;
    float previous_measurement;
    float derivative;

    float output_min;
    float output_max;

    float integrator_min;
    float integrator_max;

    /** Derivative low-pass time constant in seconds; 0 disables filtering. */
    float derivative_filter_tau;

    bool initialized;
} pid_t;

void pid_init(pid_t *pid, float kp, float ki, float kd, float output_min, float output_max);
void pid_reset(pid_t *pid);

/** Set integrator clamp. By default it matches the output clamp. */
void pid_set_integrator_limits(pid_t *pid, float min_value, float max_value);

/** Set derivative filter time constant in seconds (0 = no filter). */
void pid_set_derivative_filter(pid_t *pid, float tau_s);

float pid_update(pid_t *pid, float setpoint, float measurement, float dt);

#ifdef __cplusplus
}
#endif

#endif /* LIB_MATH_PID_H */
