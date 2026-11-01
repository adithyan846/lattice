#ifndef MODULES_STATE_ESTIMATOR_ATTITUDE_ESTIMATOR_H
#define MODULES_STATE_ESTIMATOR_ATTITUDE_ESTIMATOR_H

#include <stdbool.h>

#include <lib/math/quat.h>
#include <lib/math/vec3.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    quat_t attitude;
    vec3_t gyro_bias_rad_s;
    bool initialized;
} attitude_estimator_t;

void attitude_estimator_init(attitude_estimator_t *estimator);

/**
 * Fuse one IMU (and optional mag) sample. `mag_body` may be NULL.
 * Returns 0 on success.
 */
int attitude_estimator_update(attitude_estimator_t *estimator, const vec3_t *gyro_rad_s,
                              const vec3_t *accel_mps2, const vec3_t *mag_body, float dt_s);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_STATE_ESTIMATOR_ATTITUDE_ESTIMATOR_H */
