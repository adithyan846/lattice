#include <lib/ringbuffer/ringbuffer.h>

#include <stddef.h>
#include <string.h>

void ringbuffer_init(ringbuffer_t *rb, uint8_t *storage, size_t size)
{
    if (rb == NULL || storage == NULL || size < 2) {
        return;
    }

    rb->storage = storage;
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
}

void ringbuffer_reset(ringbuffer_t *rb)
{
    if (rb == NULL) {
        return;
    }

    rb->head = 0;
    rb->tail = 0;
}

size_t ringbuffer_used(const ringbuffer_t *rb)
{
    if (rb == NULL || rb->size == 0) {
        return 0;
    }

    const size_t head = rb->head;
    const size_t tail = rb->tail;
    return (head >= tail) ? (head - tail) : (rb->size - tail + head);
}

size_t ringbuffer_free(const ringbuffer_t *rb)
{
    if (rb == NULL || rb->size == 0) {
        return 0;
    }

    return rb->size - 1 - ringbuffer_used(rb);
}

size_t ringbuffer_write(ringbuffer_t *rb, const uint8_t *data, size_t len)
{
    if (rb == NULL || data == NULL || rb->size == 0) {
        return 0;
    }

    size_t available = ringbuffer_free(rb);

    if (len > available) {
        len = available;
    }

    size_t written = 0;

    while (written < len) {
        const size_t contiguous = rb->size - rb->head;
        const size_t chunk = (len - written < contiguous) ? (len - written) : contiguous;

        memcpy(&rb->storage[rb->head], &data[written], chunk);
        rb->head = (rb->head + chunk) % rb->size;
        written += chunk;
    }

    return written;
}

size_t ringbuffer_read(ringbuffer_t *rb, uint8_t *data, size_t len)
{
    if (rb == NULL || data == NULL || rb->size == 0) {
        return 0;
    }

    const size_t available = ringbuffer_used(rb);

    if (len > available) {
        len = available;
    }

    size_t read = 0;

    while (read < len) {
        const size_t contiguous = rb->size - rb->tail;
        const size_t chunk = (len - read < contiguous) ? (len - read) : contiguous;

        memcpy(&data[read], &rb->storage[rb->tail], chunk);
        rb->tail = (rb->tail + chunk) % rb->size;
        read += chunk;
    }

    return read;
}

int ringbuffer_peek(const ringbuffer_t *rb, uint8_t *byte)
{
    if (rb == NULL || byte == NULL || ringbuffer_used(rb) == 0) {
        return -1;
    }

    *byte = rb->storage[rb->tail];
    return 0;
}
