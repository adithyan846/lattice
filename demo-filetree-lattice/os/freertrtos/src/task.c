#include <freertrtos/task.h>

frt_status_t frt_task_create(const char *name, frt_task_entry_t entry, void *argument,
                             uint32_t stack_words, uint32_t priority, frt_task_t *task_out)
{
    return xTaskCreate(entry, name, (configSTACK_DEPTH_TYPE)stack_words, argument,
                       (UBaseType_t)priority, task_out);
}

void frt_task_delete(frt_task_t task)
{
    vTaskDelete(task);
}

void frt_task_yield(void)
{
    taskYIELD();
}

void frt_task_suspend(frt_task_t task)
{
    vTaskSuspend(task);
}

void frt_task_resume(frt_task_t task)
{
    vTaskResume(task);
}

void frt_delay_ms(uint32_t delay_ms)
{
    if (delay_ms == 0u) {
        taskYIELD();
        return;
    }

    vTaskDelay(pdMS_TO_TICKS(delay_ms));
}

uint32_t frt_task_stack_high_water(frt_task_t task)
{
    return (uint32_t)uxTaskGetStackHighWaterMark(task);
}

frt_task_t frt_task_current(void)
{
    return xTaskGetCurrentTaskHandle();
}
