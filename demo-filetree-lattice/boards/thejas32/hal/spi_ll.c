#include <hal/spi.h>

/* TODO: implement against the THEJAS32 SPI peripheral. */

int hal_spi_init(hal_spi_bus_t bus, uint32_t clock_hz, uint8_t mode)
{
    (void)bus;
    (void)clock_hz;
    (void)mode;
    return -1;
}

int hal_spi_transfer(hal_spi_bus_t bus, const uint8_t *tx, uint8_t *rx, size_t length,
                     uint32_t timeout_ms)
{
    (void)bus;
    (void)tx;
    (void)rx;
    (void)length;
    (void)timeout_ms;
    return -1;
}

int hal_spi_write(hal_spi_bus_t bus, const uint8_t *tx, size_t length, uint32_t timeout_ms)
{
    return hal_spi_transfer(bus, tx, 0, length, timeout_ms);
}

int hal_spi_read(hal_spi_bus_t bus, uint8_t *rx, size_t length, uint32_t timeout_ms)
{
    return hal_spi_transfer(bus, 0, rx, length, timeout_ms);
}
