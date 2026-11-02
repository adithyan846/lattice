#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    HAL_I2C_BUS_0 = 0,
    HAL_I2C_BUS_1 = 1,
} hal_i2c_bus_t;

int hal_i2c_init(hal_i2c_bus_t bus, uint32_t freq_hz);
int hal_i2c_write(hal_i2c_bus_t bus, uint8_t addr_7bit, const uint8_t *data, uint32_t len);
int hal_i2c_read(hal_i2c_bus_t bus, uint8_t addr_7bit, uint8_t *data, uint32_t len);
int hal_i2c_write_read(hal_i2c_bus_t bus, uint8_t addr_7bit,
                       const uint8_t *write_data, uint32_t write_len,
                       uint8_t *read_data, uint32_t read_len);
int hal_i2c_deinit(hal_i2c_bus_t bus);
