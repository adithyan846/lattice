#ifndef LATTICE_IMU_H
#define LATTICE_IMU_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Axis order is X, Y, Z in the sensor frame.
 *
 * Integer units only: soft-float on the THEJAS32 toolchain returns wrong and
 * non-repeatable results (verified on hardware: the same raw counts gave
 * correct integer values and garbage floats), so no float fields are used.
 */
typedef struct
{
    int32_t gyro_mdps[3];       /* milli-degrees per second */
    int32_t accel_mg[3];        /* milli-g */

    int16_t gyro_raw[3];
    int16_t accel_raw[3];

    uint32_t timestamp_ms;
    bool valid;
} imu_data_t;

/* Interface lifecycle */
int imu_init(void);
int imu_update(void);

/* Sensor data bus operations */
void imu_publish(const imu_data_t *data);
bool imu_get_latest(imu_data_t *data);

#endif
