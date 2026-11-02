#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    HAL_UART_0 = 0,
    HAL_UART_1 = 1,
    HAL_UART_2 = 2,
} hal_uart_t;

typedef struct {
    uint32_t baud_rate;
    uint8_t data_bits;
    uint8_t stop_bits;
    bool parity;
} hal_uart_config_t;

int hal_uart_init(hal_uart_t uart, const hal_uart_config_t *config);
int hal_uart_write(hal_uart_t uart, const uint8_t *data, uint32_t len);
int hal_uart_read(hal_uart_t uart, uint8_t *data, uint32_t len, uint32_t timeout_ms);
int hal_uart_deinit(hal_uart_t uart);
