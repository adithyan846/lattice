#include <modules/state_estimator/attitude_estimator.h>

#include <stddef.h>

void attitude_estimator_init(attitude_estimator_t *estimator)
{
    if (estimator == NULL) {
        return;
    }

    estimator->attitude = quat_identity();
    estimator->gyro_bias_rad_s = vec3_zero();
    estimator->initialized = false;
}

int attitude_estimator_update(attitude_estimator_t *estimator, const vec3_t *gyro_rad_s,
                              const vec3_t *accel_mps2, const vec3_t *mag_body, float dt_s)
{
    if (estimator == NULL || gyro_rad_s == NULL || accel_mps2 == NULL || dt_s <= 0.0f) {
        return -1;
    }

    (void)mag_body;

    /* TODO: integrate body rates into the quaternion and apply an accel/mag
     * correction (Mahony/complementary) or run the EKF in ekf/ekf_attitude.c.
     * The scaffold deliberately leaves the estimator unimplemented rather
     * than shipping an untested attitude estimate. */
    estimator->initialized = false;
    return -1;
}
