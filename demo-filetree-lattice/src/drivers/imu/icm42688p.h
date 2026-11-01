#ifndef DRIVERS_IMU_ICM42688P_H
#define DRIVERS_IMU_ICM42688P_H

#include <drivers/imu/imu.h>

#ifdef __cplusplus
extern "C" {
#endif

int icm42688p_init(void);
int icm42688p_read(imu_sample_t *sample);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_IMU_ICM42688P_H */
