#pragma once
#include "hal_gpio.h"
#include "hal_i2c.h"
#include "hal_spi.h"

#define BOARD_LED_RED_PIN    ((hal_gpio_pin_t)24)
#define BOARD_LED_GREEN_PIN  ((hal_gpio_pin_t)22)
#define BOARD_LED_BLUE_PIN   ((hal_gpio_pin_t)23)

#define BOARD_I2C_IMU_BUS    HAL_I2C_BUS_0
#define BOARD_SPI_FLASH_BUS  HAL_SPI_BUS_0

void board_init(void);
