#ifndef FREERTRTOS_TASK_H
#define FREERTRTOS_TASK_H

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TaskHandle_t frt_task_t;
typedef BaseType_t frt_status_t;
typedef void (*frt_task_entry_t)(void *argument);

#define FRT_OK    pdPASS
#define FRT_ERROR pdFAIL
#define FRT_NO_WAIT 0u
#define FRT_WAIT_FOREVER 0xFFFFFFFFu

frt_status_t frt_task_create(const char *name, frt_task_entry_t entry, void *argument,
                             uint32_t stack_words, uint32_t priority, frt_task_t *task_out);

void frt_task_delete(frt_task_t task);
void frt_task_yield(void);
void frt_task_suspend(frt_task_t task);
void frt_task_resume(frt_task_t task);

/** Delay the calling task. A zero delay yields the CPU. */
void frt_delay_ms(uint32_t delay_ms);

/** Minimum free stack ever observed, in words. */
uint32_t frt_task_stack_high_water(frt_task_t task);

frt_task_t frt_task_current(void);

#ifdef __cplusplus
}
#endif

#endif /* FREERTRTOS_TASK_H */
