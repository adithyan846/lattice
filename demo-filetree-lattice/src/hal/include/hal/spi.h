#ifndef HAL_SPI_H
#define HAL_SPI_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HAL_SPI_IMU = 0,
    HAL_SPI_BARO,
    HAL_SPI_MAG,
    HAL_SPI_SD,
    HAL_SPI_COUNT
} hal_spi_bus_t;

int hal_spi_init(hal_spi_bus_t bus, uint32_t clock_hz, uint8_t mode);
int hal_spi_transfer(hal_spi_bus_t bus, const uint8_t *tx, uint8_t *rx, size_t length,
                     uint32_t timeout_ms);
int hal_spi_write(hal_spi_bus_t bus, const uint8_t *tx, size_t length, uint32_t timeout_ms);
int hal_spi_read(hal_spi_bus_t bus, uint8_t *rx, size_t length, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* HAL_SPI_H */
