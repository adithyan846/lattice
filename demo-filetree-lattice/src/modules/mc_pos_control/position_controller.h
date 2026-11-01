#ifndef MODULES_MC_POS_CONTROL_POSITION_CONTROLLER_H
#define MODULES_MC_POS_CONTROL_POSITION_CONTROLLER_H

#include <lib/math/quat.h>
#include <lib/math/vec3.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    /** Position error to velocity setpoint gain, 1/s. */
    float kp_xy;
    float kp_z;

    /** Velocity error to acceleration gain, 1/s. */
    float kp_vel_xy;
    float kp_vel_z;

    float max_velocity_xy_mps;
    float max_velocity_z_mps;
    float max_tilt_rad;

    /** Normalised collective thrust at hover (matches the mixer scale). */
    float hover_thrust;
} position_controller_config_t;

typedef struct {
    position_controller_config_t config;
} position_controller_t;

void position_controller_default_config(position_controller_config_t *config);
void position_controller_init(position_controller_t *controller,
                              const position_controller_config_t *config);

/**
 * Cascaded position -> velocity -> acceleration controller.
 *
 * Produces a thrust vector direction as an attitude setpoint plus normalised
 * collective thrust. `velocity_setpoint` may be zero for pure position hold.
 * Returns 0 on success, -1 on invalid input.
 */
int position_controller_update(position_controller_t *controller, vec3_t position_setpoint,
                               vec3_t velocity_setpoint, vec3_t position, vec3_t velocity,
                               float yaw_setpoint, vec3_t *velocity_output, quat_t *attitude_setpoint,
                               float *thrust);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_MC_POS_CONTROL_POSITION_CONTROLLER_H */
