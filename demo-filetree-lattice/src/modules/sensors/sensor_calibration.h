#ifndef MODULES_SENSORS_SENSOR_CALIBRATION_H
#define MODULES_SENSORS_SENSOR_CALIBRATION_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float accel_bias[3];
    float accel_scale[3];
    float gyro_bias[3];
    float mag_bias[3];
    float mag_scale[3];
} sensor_calibration_t;

void sensor_calibration_init(sensor_calibration_t *calibration);

void sensor_calibration_apply_imu(const sensor_calibration_t *calibration,
                                  const float raw_accel[3], const float raw_gyro[3],
                                  float accel[3], float gyro[3]);

void sensor_calibration_apply_mag(const sensor_calibration_t *calibration,
                                  const float raw_mag[3], float mag[3]);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_SENSORS_SENSOR_CALIBRATION_H */
