#ifndef MODULES_MC_ATT_CONTROL_MC_ATT_CONTROL_H
#define MODULES_MC_ATT_CONTROL_MC_ATT_CONTROL_H

#include <stdint.h>

#include <modules/mc_att_control/attitude_controller.h>
#include <modules/mc_att_control/rate_controller.h>
#include <msg/generated/uorb_topics.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    attitude_controller_t attitude;
    rate_controller_t rate;
} mc_att_control_t;

void mc_att_control_init(mc_att_control_t *control);

/**
 * One attitude/rate control step.
 *
 * Reads the attitude, attitude setpoint and measured body rates, then produces
 * a body-rate setpoint (for logging/outer loops) and a normalised torque
 * setpoint. Thrust is passed through from the attitude setpoint.
 *
 * Returns 0 on success, -1 on invalid input or stale timestamps.
 */
int mc_att_control_step(mc_att_control_t *control, const vehicle_attitude_s *attitude,
                        const vehicle_attitude_setpoint_s *attitude_setpoint,
                        const vehicle_angular_velocity_s *angular_velocity, float dt_s,
                        vehicle_rates_setpoint_s *rates_setpoint,
                        vehicle_torque_setpoint_s *torque_setpoint,
                        vehicle_thrust_setpoint_s *thrust_setpoint);

int mc_att_control_start(void);
int mc_att_control_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_MC_ATT_CONTROL_MC_ATT_CONTROL_H */
