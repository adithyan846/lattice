#include <freertrtos/critical.h>

#include "FreeRTOS.h"
#include "task.h"

void frt_critical_enter(void)
{
    taskENTER_CRITICAL();
}

void frt_critical_exit(void)
{
    taskEXIT_CRITICAL();
}
