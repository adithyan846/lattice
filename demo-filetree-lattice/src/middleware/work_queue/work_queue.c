#include <middleware/work_queue/work_queue.h>

/* TODO: implement a low-priority FreeRTOS worker task with a delayed-item
 * queue. Use it for parameter flash writes, mission persistence and shell
 * commands. Never put flight-control work on this queue. */

int work_queue_init(void)
{
    return -1;
}

int work_queue_schedule(work_item_fn_t function, void *argument, uint32_t delay_ms)
{
    (void)function;
    (void)argument;
    (void)delay_ms;
    return -1;
}
