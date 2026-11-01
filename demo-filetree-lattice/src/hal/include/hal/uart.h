#ifndef HAL_UART_H
#define HAL_UART_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Logical UARTs. The board maps these to physical peripherals/pins. */
typedef enum {
    HAL_UART_CONSOLE = 0,
    HAL_UART_GNSS,
    HAL_UART_TELEMETRY,
    HAL_UART_RC,
    HAL_UART_COUNT
} hal_uart_bus_t;

typedef enum {
    HAL_UART_PARITY_NONE = 0,
    HAL_UART_PARITY_EVEN,
    HAL_UART_PARITY_ODD,
} hal_uart_parity_t;

typedef struct {
    uint32_t baudrate;
    uint8_t data_bits; /**< 7 or 8 */
    uint8_t stop_bits; /**< 1 or 2 */
    hal_uart_parity_t parity;
} hal_uart_config_t;

/** Called from the UART receive path. Keep it short and ISR-safe. */
typedef void (*hal_uart_rx_callback_t)(void *argument, const uint8_t *data, size_t length);

int hal_uart_init(hal_uart_bus_t bus, const hal_uart_config_t *config);
int hal_uart_write(hal_uart_bus_t bus, const uint8_t *data, size_t length,
                   uint32_t timeout_ms);
int hal_uart_read(hal_uart_bus_t bus, uint8_t *data, size_t length, uint32_t timeout_ms);
int hal_uart_set_rx_callback(hal_uart_bus_t bus, hal_uart_rx_callback_t callback,
                             void *argument);
void hal_uart_flush(hal_uart_bus_t bus);

#ifdef __cplusplus
}
#endif

#endif /* HAL_UART_H */
