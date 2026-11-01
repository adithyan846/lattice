#ifndef MODULES_MC_ATT_CONTROL_RATE_CONTROLLER_H
#define MODULES_MC_ATT_CONTROL_RATE_CONTROLLER_H

#include <stdbool.h>

#include <lib/math/pid.h>
#include <lib/math/vec3.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    /** Per-axis gains, output units are normalised torque. */
    float kp[3];
    float ki[3];
    float kd[3];
    float torque_limit;
    float integrator_limit;
    float derivative_filter_tau;
} rate_controller_config_t;

typedef struct {
    rate_controller_config_t config;
    pid_t axis[3];
    bool initialized;
} rate_controller_t;

void rate_controller_default_config(rate_controller_config_t *config);
void rate_controller_init(rate_controller_t *controller, const rate_controller_config_t *config);
void rate_controller_reset(rate_controller_t *controller);

/**
 * Body-rate PID controller. Inputs and outputs are in rad/s and normalised
 * torque; `dt_s` must be positive. Returns 0 on success.
 */
int rate_controller_update(rate_controller_t *controller, vec3_t rate_setpoint,
                           vec3_t rate_measurement, float dt_s, vec3_t *torque);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_MC_ATT_CONTROL_RATE_CONTROLLER_H */
