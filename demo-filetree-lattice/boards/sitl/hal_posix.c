#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hal/hal.h>

/* UART */
int hal_uart_init(hal_uart_bus_t bus, const hal_uart_config_t *config)
{
    (void)bus;
    (void)config;
    return 0;
}

int hal_uart_write(hal_uart_bus_t bus, const uint8_t *data, size_t length,
                   uint32_t timeout_ms)
{
    (void)timeout_ms;

    if (bus == HAL_UART_CONSOLE && data != NULL && length > 0) {
        fwrite(data, 1, length, stdout);
        fflush(stdout);
    }

    return 0;
}

int hal_uart_read(hal_uart_bus_t bus, uint8_t *data, size_t length, uint32_t timeout_ms)
{
    (void)bus;
    (void)data;
    (void)length;
    (void)timeout_ms;
    return -1;
}

int hal_uart_set_rx_callback(hal_uart_bus_t bus, hal_uart_rx_callback_t callback,
                             void *argument)
{
    (void)bus;
    (void)callback;
    (void)argument;
    return -1;
}

void hal_uart_flush(hal_uart_bus_t bus)
{
    (void)bus;
    fflush(stdout);
}

/* SPI */
int hal_spi_init(hal_spi_bus_t bus, uint32_t clock_hz, uint8_t mode)
{
    (void)bus;
    (void)clock_hz;
    (void)mode;
    return 0;
}

int hal_spi_transfer(hal_spi_bus_t bus, const uint8_t *tx, uint8_t *rx, size_t length,
                     uint32_t timeout_ms)
{
    (void)bus;
    (void)tx;
    (void)timeout_ms;

    if (rx != NULL) {
        memset(rx, 0, length);
    }

    return 0;
}

int hal_spi_write(hal_spi_bus_t bus, const uint8_t *tx, size_t length, uint32_t timeout_ms)
{
    return hal_spi_transfer(bus, tx, NULL, length, timeout_ms);
}

int hal_spi_read(hal_spi_bus_t bus, uint8_t *rx, size_t length, uint32_t timeout_ms)
{
    return hal_spi_transfer(bus, NULL, rx, length, timeout_ms);
}

/* I2C */
int hal_i2c_init(hal_i2c_bus_t bus, uint32_t clock_hz)
{
    (void)bus;
    (void)clock_hz;
    return 0;
}

int hal_i2c_write(hal_i2c_bus_t bus, uint8_t address, const uint8_t *data, size_t length,
                  uint32_t timeout_ms)
{
    (void)bus;
    (void)address;
    (void)data;
    (void)length;
    (void)timeout_ms;
    return 0;
}

int hal_i2c_read(hal_i2c_bus_t bus, uint8_t address, uint8_t *data, size_t length,
                 uint32_t timeout_ms)
{
    (void)bus;
    (void)address;
    (void)timeout_ms;

    if (data != NULL) {
        memset(data, 0, length);
    }

    return 0;
}

int hal_i2c_write_read(hal_i2c_bus_t bus, uint8_t address, const uint8_t *write_data,
                       size_t write_length, uint8_t *read_data, size_t read_length,
                       uint32_t timeout_ms)
{
    (void)bus;
    (void)address;
    (void)write_data;
    (void)write_length;
    (void)timeout_ms;

    if (read_data != NULL) {
        memset(read_data, 0, read_length);
    }

    return 0;
}

/* GPIO */
int hal_gpio_init(hal_gpio_t gpio, hal_gpio_mode_t mode)
{
    (void)gpio;
    (void)mode;
    return 0;
}

int hal_gpio_write(hal_gpio_t gpio, bool value)
{
    (void)gpio;
    (void)value;
    return 0;
}

bool hal_gpio_read(hal_gpio_t gpio)
{
    (void)gpio;
    return false;
}

int hal_gpio_attach_interrupt(hal_gpio_t gpio, hal_gpio_edge_t edge,
                              hal_gpio_callback_t callback, void *argument)
{
    (void)gpio;
    (void)edge;
    (void)callback;
    (void)argument;
    return 0;
}

/* PWM */
int hal_pwm_init(void)
{
    return 0;
}

int hal_pwm_write_us(hal_pwm_channel_t channel, uint16_t pulse_us)
{
    (void)channel;
    (void)pulse_us;
    return 0;
}

int hal_pwm_write_normalized(hal_pwm_channel_t channel, float value)
{
    (void)channel;
    (void)value;
    return 0;
}

void hal_pwm_disarm_all(void)
{
}

/* ADC */
int hal_adc_init(void)
{
    return 0;
}

float hal_adc_read(hal_adc_channel_t channel)
{
    (void)channel;
    return 0.0f;
}

/* Flash */
int hal_flash_init(void)
{
    return 0;
}

int hal_flash_read(hal_flash_partition_t partition, uint32_t offset, void *data,
                   size_t length)
{
    (void)partition;
    (void)offset;
    (void)data;
    (void)length;
    return -1;
}

int hal_flash_write(hal_flash_partition_t partition, uint32_t offset, const void *data,
                    size_t length)
{
    (void)partition;
    (void)offset;
    (void)data;
    (void)length;
    return -1;
}

int hal_flash_erase(hal_flash_partition_t partition)
{
    (void)partition;
    return 0;
}

size_t hal_flash_partition_size(hal_flash_partition_t partition)
{
    (void)partition;
    return 0;
}

/* SD */
int hal_sd_init(void)
{
    return 0;
}

bool hal_sd_present(void)
{
    return false;
}

int hal_sd_read_blocks(uint32_t block, void *buffer, uint32_t block_count)
{
    (void)block;
    (void)buffer;
    (void)block_count;
    return -1;
}

int hal_sd_write_blocks(uint32_t block, const void *buffer, uint32_t block_count)
{
    (void)block;
    (void)buffer;
    (void)block_count;
    return -1;
}

uint64_t hal_sd_capacity_blocks(void)
{
    return 0;
}

/* Reset */
void hal_system_reset(void)
{
    exit(0);
}

uint32_t hal_reset_cause(void)
{
    return 0;
}
