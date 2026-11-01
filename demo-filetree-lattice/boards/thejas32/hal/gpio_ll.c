#include <hal/gpio.h>

/* TODO: implement against the THEJAS32 GPIO controller. */

int hal_gpio_init(hal_gpio_t gpio, hal_gpio_mode_t mode)
{
    (void)gpio;
    (void)mode;
    return -1;
}

int hal_gpio_write(hal_gpio_t gpio, bool value)
{
    (void)gpio;
    (void)value;
    return -1;
}

bool hal_gpio_read(hal_gpio_t gpio)
{
    (void)gpio;
    return false;
}

int hal_gpio_attach_interrupt(hal_gpio_t gpio, hal_gpio_edge_t edge,
                              hal_gpio_callback_t callback, void *argument)
{
    (void)gpio;
    (void)edge;
    (void)callback;
    (void)argument;
    return -1;
}
