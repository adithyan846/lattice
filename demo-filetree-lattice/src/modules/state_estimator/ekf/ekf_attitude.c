#include <modules/state_estimator/ekf/ekf_attitude.h>

#include <stddef.h>

void ekf_attitude_init(ekf_attitude_t *ekf)
{
    (void)ekf;
}

int ekf_attitude_predict(ekf_attitude_t *ekf, const float gyro[3], float dt_s)
{
    (void)ekf;
    (void)gyro;
    (void)dt_s;
    return -1;
}

int ekf_attitude_update_accel(ekf_attitude_t *ekf, const float accel[3])
{
    (void)ekf;
    (void)accel;
    return -1;
}
