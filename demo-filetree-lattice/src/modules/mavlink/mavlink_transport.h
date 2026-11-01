#ifndef MODULES_MAVLINK_MAVLINK_TRANSPORT_H
#define MODULES_MAVLINK_MAVLINK_TRANSPORT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Byte-stream transport used by the MAVLink module. The telemetry UART driver
 * provides the concrete implementation; SITL supplies a UDP-backed one.
 *
 * `read` returns the number of bytes read (0 if none available) and `write`
 * returns the number of bytes written. Neither function should block the
 * control loop.
 */
typedef struct {
    void *context;
    int (*read)(void *context, uint8_t *buffer, size_t capacity);
    int (*write)(void *context, const uint8_t *buffer, size_t length);
} mavlink_transport_t;

/** Write the whole buffer, retrying short writes. Returns 0 on success. */
int mavlink_transport_write_all(const mavlink_transport_t *transport, const uint8_t *buffer,
                                size_t length);

/** Read up to `capacity` bytes. Returns bytes read or -1 on error. */
int mavlink_transport_read(const mavlink_transport_t *transport, uint8_t *buffer, size_t capacity);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_MAVLINK_MAVLINK_TRANSPORT_H */
