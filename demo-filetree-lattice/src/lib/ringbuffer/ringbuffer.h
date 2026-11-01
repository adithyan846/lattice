#ifndef LIB_RINGBUFFER_RINGBUFFER_H
#define LIB_RINGBUFFER_RINGBUFFER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Single-producer / single-consumer byte ring buffer.
 *
 * One byte is reserved to distinguish full from empty, so capacity is
 * `size - 1`. The caller owns the backing storage and must keep it alive.
 */
typedef struct {
    uint8_t *storage;
    size_t size;
    volatile size_t head;
    volatile size_t tail;
} ringbuffer_t;

void ringbuffer_init(ringbuffer_t *rb, uint8_t *storage, size_t size);
void ringbuffer_reset(ringbuffer_t *rb);
size_t ringbuffer_write(ringbuffer_t *rb, const uint8_t *data, size_t len);
size_t ringbuffer_read(ringbuffer_t *rb, uint8_t *data, size_t len);
size_t ringbuffer_used(const ringbuffer_t *rb);
size_t ringbuffer_free(const ringbuffer_t *rb);
int ringbuffer_peek(const ringbuffer_t *rb, uint8_t *byte);

#ifdef __cplusplus
}
#endif

#endif /* LIB_RINGBUFFER_RINGBUFFER_H */
