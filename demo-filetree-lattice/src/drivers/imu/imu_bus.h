#ifndef DRIVERS_IMU_IMU_BUS_H
#define DRIVERS_IMU_IMU_BUS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int imu_bus_init(void);
int imu_bus_write_register(uint8_t reg, uint8_t value);
int imu_bus_read_registers(uint8_t reg, uint8_t *data, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_IMU_IMU_BUS_H */
