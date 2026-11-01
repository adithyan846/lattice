#ifndef DRIVERS_TELEMETRY_TELEMETRY_UART_H
#define DRIVERS_TELEMETRY_TELEMETRY_UART_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int telemetry_uart_init(uint32_t baudrate);
int telemetry_uart_write(const uint8_t *data, size_t length, uint32_t timeout_ms);
int telemetry_uart_read(uint8_t *data, size_t length, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_TELEMETRY_TELEMETRY_UART_H */
