#include "hal_gpio.h"
#include "hal_errno.h"
#include "stdlib.h"
#include "config.h"
#include "gpio.h"

#define GPIO_MAX_PIN 31

int hal_gpio_init(void) { return 0; }

int hal_gpio_set_dir(hal_gpio_pin_t pin, hal_gpio_dir_t dir) {
    (void)dir;
    if (pin > GPIO_MAX_PIN) return -EINVAL;
    return 0;
}

int hal_gpio_write(hal_gpio_pin_t pin, hal_gpio_level_t level) {
    if (pin > GPIO_MAX_PIN) return -EINVAL;
    GPIO_write_pin((US)pin, (US)level);
    return 0;
}

int hal_gpio_read(hal_gpio_pin_t pin, hal_gpio_level_t *out) {
    if (pin > GPIO_MAX_PIN || !out) return -EINVAL;
    US val = GPIO_read_pin((US)pin);
    *out = (hal_gpio_level_t)val;
    return 0;
}

int hal_gpio_toggle(hal_gpio_pin_t pin) {
    hal_gpio_level_t level;
    int ret = hal_gpio_read(pin, &level);
    if (ret) return ret;
    return hal_gpio_write(pin, level == HAL_GPIO_LEVEL_HIGH ? HAL_GPIO_LEVEL_LOW : HAL_GPIO_LEVEL_HIGH);
}

int hal_gpio_pulse_duration(hal_gpio_pin_t pin, hal_gpio_level_t level, uint32_t *out_us) {
    if (pin > GPIO_MAX_PIN || !out_us) return -EINVAL;
    *out_us = (uint32_t)pulse_duration((US)pin, (US)level);
    return 0;
}
