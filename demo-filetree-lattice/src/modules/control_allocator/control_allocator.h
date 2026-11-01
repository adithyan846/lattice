#ifndef MODULES_CONTROL_ALLOCATOR_CONTROL_ALLOCATOR_H
#define MODULES_CONTROL_ALLOCATOR_CONTROL_ALLOCATOR_H

#include <modules/control_allocator/actuator_limits.h>
#include <modules/control_allocator/mixer_quad_x.h>
#include <msg/generated/uorb_topics.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    mixer_quad_x_config_t mixer;
    actuator_limits_t limits;
} control_allocator_t;

void control_allocator_init(control_allocator_t *allocator);

/**
 * Allocate torque/thrust to the four motors and publish-ready actuator_motors.
 * Slew limiting is applied unless `dt_s` is zero. Returns 0 on success.
 */
int control_allocator_step(control_allocator_t *allocator,
                           const vehicle_torque_setpoint_s *torque_setpoint,
                           const vehicle_thrust_setpoint_s *thrust_setpoint, float dt_s,
                           actuator_motors_s *motors);

int control_allocator_start(void);
int control_allocator_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_CONTROL_ALLOCATOR_CONTROL_ALLOCATOR_H */
