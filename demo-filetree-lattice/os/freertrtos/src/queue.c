#include <freertrtos/queue.h>

frt_queue_t frt_queue_create(size_t item_size, size_t item_count)
{
    return xQueueCreate((UBaseType_t)item_count, (UBaseType_t)item_size);
}

void frt_queue_delete(frt_queue_t queue)
{
    vQueueDelete(queue);
}

frt_status_t frt_queue_send(frt_queue_t queue, const void *item, uint32_t timeout_ms)
{
    return xQueueSend(queue, item, pdMS_TO_TICKS(timeout_ms));
}

frt_status_t frt_queue_receive(frt_queue_t queue, void *item, uint32_t timeout_ms)
{
    return xQueueReceive(queue, item, pdMS_TO_TICKS(timeout_ms));
}

frt_status_t frt_queue_send_from_isr(frt_queue_t queue, const void *item,
                                     BaseType_t *higher_priority_task_woken)
{
    return xQueueSendFromISR(queue, item, higher_priority_task_woken);
}

uint32_t frt_queue_messages_waiting(frt_queue_t queue)
{
    return (uint32_t)uxQueueMessagesWaiting(queue);
}
