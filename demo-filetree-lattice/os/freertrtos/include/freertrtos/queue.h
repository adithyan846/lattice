#ifndef FREERTRTOS_QUEUE_H
#define FREERTRTOS_QUEUE_H

#include <stddef.h>
#include <stdint.h>

#include <freertrtos/task.h>

#include "FreeRTOS.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef QueueHandle_t frt_queue_t;

frt_queue_t frt_queue_create(size_t item_size, size_t item_count);
void frt_queue_delete(frt_queue_t queue);

/** Copy an item into the queue. timeout_ms = 0 for non-blocking. */
frt_status_t frt_queue_send(frt_queue_t queue, const void *item, uint32_t timeout_ms);

/** Copy an item out of the queue. timeout_ms = 0 for non-blocking. */
frt_status_t frt_queue_receive(frt_queue_t queue, void *item, uint32_t timeout_ms);

/** Send from an ISR. Returns pdTRUE if a higher-priority task was woken. */
frt_status_t frt_queue_send_from_isr(frt_queue_t queue, const void *item,
                                     BaseType_t *higher_priority_task_woken);

uint32_t frt_queue_messages_waiting(frt_queue_t queue);

#ifdef __cplusplus
}
#endif

#endif /* FREERTRTOS_QUEUE_H */
