#include <drivers/telemetry/telemetry_uart.h>

#include <hal/uart.h>

int telemetry_uart_init(uint32_t baudrate)
{
    const hal_uart_config_t config = {
        .baudrate = baudrate,
        .data_bits = 8u,
        .stop_bits = 1u,
        .parity = HAL_UART_PARITY_NONE,
    };

    return hal_uart_init(HAL_UART_TELEMETRY, &config);
}

int telemetry_uart_write(const uint8_t *data, size_t length, uint32_t timeout_ms)
{
    return hal_uart_write(HAL_UART_TELEMETRY, data, length, timeout_ms);
}

int telemetry_uart_read(uint8_t *data, size_t length, uint32_t timeout_ms)
{
    return hal_uart_read(HAL_UART_TELEMETRY, data, length, timeout_ms);
}
