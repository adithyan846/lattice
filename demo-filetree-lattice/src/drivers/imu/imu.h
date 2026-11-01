#ifndef DRIVERS_IMU_IMU_H
#define DRIVERS_IMU_IMU_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float accel_mps2[3];
    float gyro_rad_s[3];
    float temperature_c;
    uint8_t clip_count;
    uint64_t timestamp_us;
} imu_sample_t;

typedef struct {
    const char *name;
    int (*init)(void);
    int (*read)(imu_sample_t *sample);
} imu_driver_t;

const imu_driver_t *imu_icm42688p_get(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_IMU_IMU_H */
