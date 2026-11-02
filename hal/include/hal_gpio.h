#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef uint16_t hal_gpio_pin_t;

typedef enum {
    HAL_GPIO_DIR_INPUT  = 0,
    HAL_GPIO_DIR_OUTPUT = 1,
} hal_gpio_dir_t;

typedef enum {
    HAL_GPIO_LEVEL_LOW  = 0,
    HAL_GPIO_LEVEL_HIGH = 1,
} hal_gpio_level_t;

int hal_gpio_init(void);
int hal_gpio_set_dir(hal_gpio_pin_t pin, hal_gpio_dir_t dir);
int hal_gpio_write(hal_gpio_pin_t pin, hal_gpio_level_t level);
int hal_gpio_read(hal_gpio_pin_t pin, hal_gpio_level_t *out_level);
int hal_gpio_toggle(hal_gpio_pin_t pin);
int hal_gpio_pulse_duration(hal_gpio_pin_t pin, hal_gpio_level_t level, uint32_t *out_us);
