#ifndef MIDDLEWARE_WORK_QUEUE_WORK_QUEUE_H
#define MIDDLEWARE_WORK_QUEUE_WORK_QUEUE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*work_item_fn_t)(void *argument);

int work_queue_init(void);

/**
 * Schedule a function to run on the low-priority work queue. `delay_ms` is a
 * minimum delay; the worker may run it later. Returns 0 if queued.
 */
int work_queue_schedule(work_item_fn_t function, void *argument, uint32_t delay_ms);

#ifdef __cplusplus
}
#endif

#endif /* MIDDLEWARE_WORK_QUEUE_WORK_QUEUE_H */
