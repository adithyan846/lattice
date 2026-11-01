#ifndef MODULES_MC_ATT_CONTROL_ATTITUDE_CONTROLLER_H
#define MODULES_MC_ATT_CONTROL_ATTITUDE_CONTROLLER_H

#include <lib/math/quat.h>
#include <lib/math/vec3.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    /** Attitude error to body-rate proportional gain, 1/s. */
    float kp_roll;
    float kp_pitch;
    float kp_yaw;

    /** Body-rate setpoint saturation, rad/s. */
    float max_roll_rate;
    float max_pitch_rate;
    float max_yaw_rate;
} attitude_controller_config_t;

typedef struct {
    attitude_controller_config_t config;
    quat_t attitude_setpoint;
} attitude_controller_t;

/** Fill a conservative default configuration (to be tuned in SITL). */
void attitude_controller_default_config(attitude_controller_config_t *config);

void attitude_controller_init(attitude_controller_t *controller,
                              const attitude_controller_config_t *config);

/**
 * Quaternion attitude P controller.
 *
 * `attitude` and `attitude_setpoint` map body to world. The output is a body
 * rate setpoint. `yaw_rate_ff` is added to the yaw axis for coordinated turns.
 * Returns 0 on success, -1 on invalid input.
 */
int attitude_controller_update(attitude_controller_t *controller, quat_t attitude,
                               quat_t attitude_setpoint, float yaw_rate_ff, vec3_t *rate_setpoint);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_MC_ATT_CONTROL_ATTITUDE_CONTROLLER_H */
