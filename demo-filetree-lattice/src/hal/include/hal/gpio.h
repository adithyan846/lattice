#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HAL_GPIO_LED_STATUS = 0,
    HAL_GPIO_LED_ARMED,
    HAL_GPIO_SAFETY_SWITCH,
    HAL_GPIO_KILL_SWITCH,
    HAL_GPIO_IMU_DRDY,
    HAL_GPIO_BARO_DRDY,
    HAL_GPIO_SD_DETECT,
    HAL_GPIO_COUNT
} hal_gpio_t;

typedef enum {
    HAL_GPIO_MODE_INPUT = 0,
    HAL_GPIO_MODE_INPUT_PULLUP,
    HAL_GPIO_MODE_INPUT_PULLDOWN,
    HAL_GPIO_MODE_OUTPUT,
} hal_gpio_mode_t;

typedef enum {
    HAL_GPIO_EDGE_RISING = 0,
    HAL_GPIO_EDGE_FALLING,
    HAL_GPIO_EDGE_BOTH,
} hal_gpio_edge_t;

typedef void (*hal_gpio_callback_t)(void *argument);

int hal_gpio_init(hal_gpio_t gpio, hal_gpio_mode_t mode);
int hal_gpio_write(hal_gpio_t gpio, bool value);
bool hal_gpio_read(hal_gpio_t gpio);
int hal_gpio_attach_interrupt(hal_gpio_t gpio, hal_gpio_edge_t edge,
                              hal_gpio_callback_t callback, void *argument);

#ifdef __cplusplus
}
#endif

#endif /* HAL_GPIO_H */
