#include <modules/sensors/sensor_calibration.h>

#include <stddef.h>

void sensor_calibration_init(sensor_calibration_t *calibration)
{
    if (calibration == NULL) {
        return;
    }

    for (int i = 0; i < 3; i++) {
        calibration->accel_bias[i] = 0.0f;
        calibration->accel_scale[i] = 1.0f;
        calibration->gyro_bias[i] = 0.0f;
        calibration->mag_bias[i] = 0.0f;
        calibration->mag_scale[i] = 1.0f;
    }
}

void sensor_calibration_apply_imu(const sensor_calibration_t *calibration,
                                  const float raw_accel[3], const float raw_gyro[3],
                                  float accel[3], float gyro[3])
{
    if (calibration == NULL || raw_accel == NULL || raw_gyro == NULL || accel == NULL ||
        gyro == NULL) {
        return;
    }

    for (int i = 0; i < 3; i++) {
        accel[i] = (raw_accel[i] - calibration->accel_bias[i]) * calibration->accel_scale[i];
        gyro[i] = raw_gyro[i] - calibration->gyro_bias[i];
    }
}

void sensor_calibration_apply_mag(const sensor_calibration_t *calibration,
                                  const float raw_mag[3], float mag[3])
{
    if (calibration == NULL || raw_mag == NULL || mag == NULL) {
        return;
    }

    for (int i = 0; i < 3; i++) {
        mag[i] = (raw_mag[i] - calibration->mag_bias[i]) * calibration->mag_scale[i];
    }
}
