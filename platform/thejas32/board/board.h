#pragma once
#include "hal_gpio.h"
#include "hal_i2c.h"
#include "hal_spi.h"

#define BOARD_LED_RED_PIN    ((hal_gpio_pin_t)24)
#define BOARD_LED_GREEN_PIN  ((hal_gpio_pin_t)22)
#define BOARD_LED_BLUE_PIN   ((hal_gpio_pin_t)23)

#define BOARD_I2C_IMU_BUS    HAL_I2C_BUS_0
#define BOARD_SPI_FLASH_BUS  HAL_SPI_BUS_0

/* LSM6DSO on header J7: SPI1CLK / SPI1MOSI / SPI1MISO / SPI1CSN (CS0) */
#define BOARD_SPI_IMU_BUS    HAL_SPI_BUS_1

/* BMP280 on header J4: SPI2CLK / SPI2MOSI / SPI2MISO / SPI2CSN (CS0) */
#define BOARD_SPI_BARO_BUS   HAL_SPI_BUS_2

/*
 * Bring up board GPIO and the sensor SPI buses.
 * Returns 0 on success, or the first negative HAL error.
 */
int board_init(void);
