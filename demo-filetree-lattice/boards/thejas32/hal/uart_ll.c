#include <hal/uart.h>

/* TODO: implement against the THEJAS32 UART peripheral. */

int hal_uart_init(hal_uart_bus_t bus, const hal_uart_config_t *config)
{
    (void)bus;
    (void)config;
    return -1;
}

int hal_uart_write(hal_uart_bus_t bus, const uint8_t *data, size_t length,
                   uint32_t timeout_ms)
{
    (void)bus;
    (void)data;
    (void)length;
    (void)timeout_ms;
    return -1;
}

int hal_uart_read(hal_uart_bus_t bus, uint8_t *data, size_t length, uint32_t timeout_ms)
{
    (void)bus;
    (void)data;
    (void)length;
    (void)timeout_ms;
    return -1;
}

int hal_uart_set_rx_callback(hal_uart_bus_t bus, hal_uart_rx_callback_t callback,
                             void *argument)
{
    (void)bus;
    (void)callback;
    (void)argument;
    return -1;
}

void hal_uart_flush(hal_uart_bus_t bus)
{
    (void)bus;
}
