#include <freertrtos/semphr.h>

frt_mutex_t frt_mutex_create(void)
{
    return xSemaphoreCreateMutex();
}

void frt_mutex_delete(frt_mutex_t mutex)
{
    vSemaphoreDelete(mutex);
}

frt_status_t frt_mutex_take(frt_mutex_t mutex, uint32_t timeout_ms)
{
    return xSemaphoreTake(mutex, pdMS_TO_TICKS(timeout_ms));
}

frt_status_t frt_mutex_give(frt_mutex_t mutex)
{
    return xSemaphoreGive(mutex);
}

frt_sem_t frt_sem_create_binary(void)
{
    return xSemaphoreCreateBinary();
}

frt_sem_t frt_sem_create_counting(uint32_t max_count, uint32_t initial_count)
{
    return xSemaphoreCreateCounting((UBaseType_t)max_count, (UBaseType_t)initial_count);
}

void frt_sem_delete(frt_sem_t sem)
{
    vSemaphoreDelete(sem);
}

frt_status_t frt_sem_wait(frt_sem_t sem, uint32_t timeout_ms)
{
    return xSemaphoreTake(sem, pdMS_TO_TICKS(timeout_ms));
}

frt_status_t frt_sem_signal(frt_sem_t sem)
{
    return xSemaphoreGive(sem);
}

frt_status_t frt_sem_signal_from_isr(frt_sem_t sem, BaseType_t *higher_priority_task_woken)
{
    return xSemaphoreGiveFromISR(sem, higher_priority_task_woken);
}
