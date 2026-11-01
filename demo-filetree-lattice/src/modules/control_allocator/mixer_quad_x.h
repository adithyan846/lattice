#ifndef MODULES_CONTROL_ALLOCATOR_MIXER_QUAD_X_H
#define MODULES_CONTROL_ALLOCATOR_MIXER_QUAD_X_H

#include <lib/math/vec3.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MIXER_QUAD_X_MOTOR_COUNT 4

/**
 * Motor order:
 *   0: front-right  (CCW)
 *   1: rear-left    (CCW)
 *   2: front-left   (CW)
 *   3: rear-right   (CW)
 *
 * Body frame is NED-style: x forward, y right, z down. Positive roll is right
 * wing down, positive pitch is nose up and positive yaw is nose right.
 */
typedef struct {
    /** Normalised torque command to motor delta gain. */
    float torque_gain;
    float motor_min;
    float motor_max;
} mixer_quad_x_config_t;

void mixer_quad_x_default_config(mixer_quad_x_config_t *config);

/**
 * Mix normalised torque and collective thrust into four motor commands.
 *
 * `torque` is in [-1, 1] per axis and `thrust` in [0, 1] (0.5 hovers). The
 * output is clamped to [motor_min, motor_max]. Returns 0 on success.
 */
int mixer_quad_x_mix(const mixer_quad_x_config_t *config, vec3_t torque, float thrust,
                     float motors[MIXER_QUAD_X_MOTOR_COUNT]);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_CONTROL_ALLOCATOR_MIXER_QUAD_X_H */
