#ifndef MODULES_STATE_ESTIMATOR_EKF_EKF_ATTITUDE_H
#define MODULES_STATE_ESTIMATOR_EKF_EKF_ATTITUDE_H

#include <stdbool.h>

#include <lib/math/quat.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    quat_t q;
    float gyro_bias[3];
    float covariance[36];
    bool initialized;
} ekf_attitude_t;

void ekf_attitude_init(ekf_attitude_t *ekf);
int ekf_attitude_predict(ekf_attitude_t *ekf, const float gyro[3], float dt_s);
int ekf_attitude_update_accel(ekf_attitude_t *ekf, const float accel[3]);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_STATE_ESTIMATOR_EKF_EKF_ATTITUDE_H */
