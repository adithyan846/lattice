#ifndef MODULES_STATE_ESTIMATOR_EKF_EKF_POSITION_H
#define MODULES_STATE_ESTIMATOR_EKF_EKF_POSITION_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EKF_POSITION_STATE_SIZE 15

typedef struct {
    float state[EKF_POSITION_STATE_SIZE];
    float covariance[EKF_POSITION_STATE_SIZE * EKF_POSITION_STATE_SIZE];
    bool initialized;
} ekf_position_t;

void ekf_position_init(ekf_position_t *ekf);
int ekf_position_predict(ekf_position_t *ekf, const float accel_ned[3], float dt_s);
int ekf_position_update_baro(ekf_position_t *ekf, float altitude_m);
int ekf_position_update_gnss(ekf_position_t *ekf, double lat, double lon, float alt,
                             const float velocity_ned[3]);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_STATE_ESTIMATOR_EKF_EKF_POSITION_H */
