#include "hal_uart.h"
#include "hal_errno.h"
#include "debug_uart.h"

int hal_uart_init(hal_uart_t uart, const hal_uart_config_t *config) {
    (void)config;
    if (uart != HAL_UART_0) return -ENOTSUP;
    /* UART_0 already initialized by bootloader at 115200; do not re-init */
    return 0;
}

int hal_uart_write(hal_uart_t uart, const uint8_t *data, uint32_t len) {
    if (uart != HAL_UART_0) return -ENOTSUP;
    for (uint32_t i = 0; i < len; i++) {
        tx_uart((UC)data[i]);
    }
    return 0;
}

int hal_uart_read(hal_uart_t uart, uint8_t *data, uint32_t len, uint32_t timeout_ms) {
    (void)timeout_ms;
    if (uart != HAL_UART_0 || !data) return -EINVAL;
    for (uint32_t i = 0; i < len; i++) {
        data[i] = (uint8_t)rx_uart();
    }
    return 0;
}

int hal_uart_deinit(hal_uart_t uart) { (void)uart; return 0; }