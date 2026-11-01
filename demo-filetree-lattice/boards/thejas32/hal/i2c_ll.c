#include <hal/i2c.h>

/* TODO: implement against the THEJAS32 I2C peripheral. */

int hal_i2c_init(hal_i2c_bus_t bus, uint32_t clock_hz)
{
    (void)bus;
    (void)clock_hz;
    return -1;
}

int hal_i2c_write(hal_i2c_bus_t bus, uint8_t address, const uint8_t *data, size_t length,
                  uint32_t timeout_ms)
{
    (void)bus;
    (void)address;
    (void)data;
    (void)length;
    (void)timeout_ms;
    return -1;
}

int hal_i2c_read(hal_i2c_bus_t bus, uint8_t address, uint8_t *data, size_t length,
                 uint32_t timeout_ms)
{
    (void)bus;
    (void)address;
    (void)data;
    (void)length;
    (void)timeout_ms;
    return -1;
}

int hal_i2c_write_read(hal_i2c_bus_t bus, uint8_t address, const uint8_t *write_data,
                       size_t write_length, uint8_t *read_data, size_t read_length,
                       uint32_t timeout_ms)
{
    (void)bus;
    (void)address;
    (void)write_data;
    (void)write_length;
    (void)read_data;
    (void)read_length;
    (void)timeout_ms;
    return -1;
}
