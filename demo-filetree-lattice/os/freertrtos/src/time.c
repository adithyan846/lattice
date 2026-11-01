#include <freertrtos/time.h>

#include "FreeRTOS.h"
#include "task.h"

uint64_t frt_time_us(void)
{
    const TickType_t ticks = xTaskGetTickCount();
    return ((uint64_t)ticks * 1000000ULL) / (uint64_t)configTICK_RATE_HZ;
}

uint64_t frt_time_ms(void)
{
    return (uint64_t)xTaskGetTickCount() * (uint64_t)portTICK_PERIOD_MS;
}

uint32_t frt_ms_to_ticks(uint32_t delay_ms)
{
    return (uint32_t)pdMS_TO_TICKS(delay_ms);
}

void frt_delay_us(uint32_t delay_us)
{
    if (delay_us == 0u) {
        return;
    }

    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        if (delay_us < 1000u) {
            const uint64_t start = frt_time_us();

            while ((frt_time_us() - start) < delay_us) {
                /* Busy-wait a fraction of a tick. */
            }

        } else {
            vTaskDelay(pdMS_TO_TICKS((delay_us + 999u) / 1000u));
        }

    } else {
        /* Pre-scheduler busy-wait. This is only used during board bring-up;
         * calibrate it against a scope if you rely on it. */
        for (volatile uint32_t i = 0; i < delay_us * 8u; i++) {
        }
    }
}
