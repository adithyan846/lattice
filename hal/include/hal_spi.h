#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    HAL_SPI_BUS_0 = 0,
    HAL_SPI_BUS_1 = 1,
    HAL_SPI_BUS_2 = 2,
    HAL_SPI_BUS_3 = 3,
    HAL_SPI_BUS_COUNT = 4
} hal_spi_bus_t;

typedef enum {
    HAL_SPI_MODE_0 = 0,
    HAL_SPI_MODE_1 = 1,
    HAL_SPI_MODE_2 = 2,
    HAL_SPI_MODE_3 = 3,
} hal_spi_mode_t;

typedef enum {
    HAL_SPI_BIT_ORDER_MSB = 0,
    HAL_SPI_BIT_ORDER_LSB = 1,
} hal_spi_bit_order_t;

typedef enum {
    HAL_SPI_CS_MODE_CSAAT  = 0,  // Manual CSAAT register control (fixed CS0)
    HAL_SPI_CS_MODE_GPIO   = 1,  // Manual GPIO pin control
    HAL_SPI_CS_MODE_NONE   = 2,  // No CS control (externally tied)
} hal_spi_cs_mode_t;

typedef enum {
    HAL_SPI_CS_HW_0 = 0,
    HAL_SPI_CS_HW_1 = 1,
    HAL_SPI_CS_HW_2 = 2,
    HAL_SPI_CS_HW_3 = 3,
} hal_spi_cs_hw_t;

typedef enum {
    HAL_SPI_BAUD_DIV_4   = 0,
    HAL_SPI_BAUD_DIV_8   = 1,
    HAL_SPI_BAUD_DIV_16  = 2,
    HAL_SPI_BAUD_DIV_32  = 3,
    HAL_SPI_BAUD_DIV_64  = 4,
    HAL_SPI_BAUD_DIV_128 = 5,
    HAL_SPI_BAUD_DIV_256 = 6,
    HAL_SPI_BAUD_DIV_512 = 7,
    HAL_SPI_BAUD_DIV_1024 = 8,
    HAL_SPI_BAUD_DIV_2048 = 9,
} hal_spi_baud_div_t;

typedef struct {
    hal_spi_mode_t       mode;
    hal_spi_bit_order_t  bit_order;
    hal_spi_cs_mode_t    cs_mode;
    union {
        uint8_t gpio_pin;
        hal_spi_cs_hw_t hw_cs;
    };
    uint32_t             freq_hz;       // Target frequency in Hz (0 = use baud_div)
    hal_spi_baud_div_t   baud_div;      // Explicit divider (used if freq_hz == 0)
    uint8_t              word_size;     // 8 only (16-bit unverified on hardware)
} hal_spi_config_t;

int hal_spi_init(hal_spi_bus_t bus, const hal_spi_config_t *config);
int hal_spi_deinit(hal_spi_bus_t bus);
int hal_spi_transfer(hal_spi_bus_t bus, const uint8_t *tx, uint8_t *rx, uint32_t len, uint32_t timeout_ms);
int hal_spi_write_read(hal_spi_bus_t bus, const uint8_t *tx, uint32_t tx_len, uint8_t *rx, uint32_t rx_len, uint32_t timeout_ms);
int hal_spi_write(hal_spi_bus_t bus, const uint8_t *data, uint32_t len, uint32_t timeout_ms);
int hal_spi_read(hal_spi_bus_t bus, uint8_t *data, uint32_t len, uint32_t timeout_ms);
int hal_spi_set_cs(hal_spi_bus_t bus, bool assert);
int hal_spi_set_frequency(hal_spi_bus_t bus, uint32_t freq_hz);
int hal_spi_set_baud_div(hal_spi_bus_t bus, hal_spi_baud_div_t div);
int hal_spi_set_mode(hal_spi_bus_t bus, hal_spi_mode_t mode);
int hal_spi_set_bit_order(hal_spi_bus_t bus, hal_spi_bit_order_t order);
int hal_spi_check_overrun(hal_spi_bus_t bus);
bool hal_spi_is_busy(hal_spi_bus_t bus);