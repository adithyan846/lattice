#include <drivers/imu/imu_bus.h>

#include <hal/gpio.h>
#include <hal/spi.h>

/* The IMU chip-select is a GPIO controlled by this bus helper. */
int imu_bus_init(void)
{
    /* TODO: configure the chip-select GPIO and SPI mode/clock. */
    return -1;
}

int imu_bus_write_register(uint8_t reg, uint8_t value)
{
    const uint8_t tx[2] = {(uint8_t)(reg & 0x7Fu), value};
    (void)hal_gpio_write(HAL_GPIO_IMU_DRDY, false);
    const int result = hal_spi_write(HAL_SPI_IMU, tx, sizeof(tx), 10);
    (void)hal_gpio_write(HAL_GPIO_IMU_DRDY, true);
    return result;
}

int imu_bus_read_registers(uint8_t reg, uint8_t *data, size_t length)
{
    if (data == NULL || length == 0) {
        return -1;
    }

    /* TODO: use the actual chip-select pin, not the data-ready pin. */
    const uint8_t address = (uint8_t)(reg | 0x80u);
    (void)hal_spi_write(HAL_SPI_IMU, &address, 1, 10);
    return hal_spi_read(HAL_SPI_IMU, data, length, 10);
}
