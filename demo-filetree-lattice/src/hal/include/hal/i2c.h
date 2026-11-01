#ifndef HAL_I2C_H
#define HAL_I2C_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HAL_I2C_MAG = 0,
    HAL_I2C_BARO,
    HAL_I2C_EXTERNAL,
    HAL_I2C_COUNT
} hal_i2c_bus_t;

int hal_i2c_init(hal_i2c_bus_t bus, uint32_t clock_hz);
int hal_i2c_write(hal_i2c_bus_t bus, uint8_t address, const uint8_t *data, size_t length,
                  uint32_t timeout_ms);
int hal_i2c_read(hal_i2c_bus_t bus, uint8_t address, uint8_t *data, size_t length,
                 uint32_t timeout_ms);
int hal_i2c_write_read(hal_i2c_bus_t bus, uint8_t address, const uint8_t *write_data,
                       size_t write_length, uint8_t *read_data, size_t read_length,
                       uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* HAL_I2C_H */
