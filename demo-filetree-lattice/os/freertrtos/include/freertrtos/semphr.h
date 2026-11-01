#ifndef FREERTRTOS_SEMPHR_H
#define FREERTRTOS_SEMPHR_H

#include <stdint.h>

#include <freertrtos/task.h>

#include "FreeRTOS.h"
#include "semphr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef SemaphoreHandle_t frt_mutex_t;
typedef SemaphoreHandle_t frt_sem_t;

frt_mutex_t frt_mutex_create(void);
void frt_mutex_delete(frt_mutex_t mutex);
frt_status_t frt_mutex_take(frt_mutex_t mutex, uint32_t timeout_ms);
frt_status_t frt_mutex_give(frt_mutex_t mutex);

frt_sem_t frt_sem_create_binary(void);
frt_sem_t frt_sem_create_counting(uint32_t max_count, uint32_t initial_count);
void frt_sem_delete(frt_sem_t sem);
frt_status_t frt_sem_wait(frt_sem_t sem, uint32_t timeout_ms);
frt_status_t frt_sem_signal(frt_sem_t sem);
frt_status_t frt_sem_signal_from_isr(frt_sem_t sem, BaseType_t *higher_priority_task_woken);

#ifdef __cplusplus
}
#endif

#endif /* FREERTRTOS_SEMPHR_H */
