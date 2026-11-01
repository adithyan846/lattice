#include <freertrtos.h>

#include "FreeRTOS.h"
#include "task.h"

int frt_init(void)
{
    /* Board code may create static kernel objects here before the scheduler
     * starts. Keep this function free of hardware access; board_init() owns
     * clocks and pins. */
    return 0;
}

void frt_start_scheduler(void)
{
    vTaskStartScheduler();

    /* vTaskStartScheduler() only returns if the idle task could not be
     * created or the heap is exhausted. Treat that as a fatal fault. */
    frt_assert_failed(__FILE__, __LINE__);
}
