#include <modules/mavlink/mavlink_transport.h>

#include <stddef.h>

int mavlink_transport_write_all(const mavlink_transport_t *transport, const uint8_t *buffer,
                                size_t length)
{
    if (transport == NULL || transport->write == NULL || (buffer == NULL && length > 0)) {
        return -1;
    }

    size_t written = 0;

    while (written < length) {
        const int result = transport->write(transport->context, buffer + written, length - written);

        if (result <= 0) {
            return -1;
        }

        written += (size_t)result;
    }

    return 0;
}

int mavlink_transport_read(const mavlink_transport_t *transport, uint8_t *buffer, size_t capacity)
{
    if (transport == NULL || transport->read == NULL || buffer == NULL) {
        return -1;
    }

    return transport->read(transport->context, buffer, capacity);
}
