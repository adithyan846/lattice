#ifndef MODULES_MC_POS_CONTROL_MC_POS_CONTROL_H
#define MODULES_MC_POS_CONTROL_MC_POS_CONTROL_H

#include <modules/mc_pos_control/position_controller.h>
#include <msg/generated/uorb_topics.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    position_controller_t controller;
} mc_pos_control_t;

void mc_pos_control_init(mc_pos_control_t *control);

/**
 * Convert a trajectory setpoint and local position into an attitude setpoint.
 * Returns 0 on success, -1 when the position estimate is invalid.
 */
int mc_pos_control_step(mc_pos_control_t *control, const vehicle_local_position_s *local_position,
                        const trajectory_setpoint_s *trajectory_setpoint,
                        vehicle_attitude_setpoint_s *attitude_setpoint);

int mc_pos_control_start(void);
int mc_pos_control_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_MC_POS_CONTROL_MC_POS_CONTROL_H */
