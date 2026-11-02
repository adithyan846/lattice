#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    HAL_SPI_BUS_0 = 0,
    HAL_SPI_BUS_1 = 1,
    HAL_SPI_BUS_2 = 2,
    HAL_SPI_BUS_3 = 3,
} hal_spi_bus_t;

typedef enum {
    HAL_SPI_MODE_0 = 0,
    HAL_SPI_MODE_1 = 1,
    HAL_SPI_MODE_2 = 2,
    HAL_SPI_MODE_3 = 3,
} hal_spi_mode_t;

typedef enum {
    HAL_SPI_CS_HARDWARE = 0xFF,
    HAL_SPI_CS_NONE     = 0xFE,
} hal_spi_cs_t;

typedef struct {
    hal_spi_mode_t mode;
    uint32_t freq_hz;
    bool msb_first;
    uint8_t cs_pin;
    uint8_t word_size;
} hal_spi_config_t;

int hal_spi_init(hal_spi_bus_t bus, const hal_spi_config_t *config);
int hal_spi_transfer(hal_spi_bus_t bus, const uint8_t *tx, uint8_t *rx, uint32_t len);
int hal_spi_write(hal_spi_bus_t bus, const uint8_t *data, uint32_t len);
int hal_spi_read(hal_spi_bus_t bus, uint8_t *data, uint32_t len);
int hal_spi_set_cs(hal_spi_bus_t bus, bool assert);
int hal_spi_deinit(hal_spi_bus_t bus);
