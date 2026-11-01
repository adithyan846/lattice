#include "board.h"

#include <stdio.h>
#include <stdlib.h>

#include <freertrtos/fault.h>

#include "FreeRTOS.h"
#include "task.h"

static StaticTask_t idle_task_buffer;
static StackType_t idle_task_stack[configMINIMAL_STACK_SIZE];

static StaticTask_t timer_task_buffer;
static StackType_t timer_task_stack[configTIMER_TASK_STACK_DEPTH];

int board_clock_init(void)
{
    return 0;
}

int board_init(void)
{
    return 0;
}

void board_reset(void)
{
    fflush(stdout);
    exit(0);
}

uint32_t board_unique_id(void)
{
    return 0x51u;
}

const char *board_name(void)
{
    return "sitl";
}

void frt_assert_failed(const char *file, int line)
{
    fprintf(stderr, "ASSERT %s:%d\n", file, line);
    fflush(stderr);
    abort();
}

void frt_stack_overflow_hook(void *task_handle, const char *task_name)
{
    (void)task_handle;
    fprintf(stderr, "STACK OVERFLOW in %s\n", task_name != NULL ? task_name : "?");
    fflush(stderr);
    abort();
}

void frt_malloc_failed_hook(void)
{
    fprintf(stderr, "MALLOC FAILED\n");
    fflush(stderr);
    abort();
}

void frt_fault_reboot(void)
{
    board_reset();
}

void vApplicationGetIdleTaskMemory(StaticTask_t **idle_task_tcb,
                                   StackType_t **idle_task_stack_buffer,
                                   uint32_t *idle_task_stack_size)
{
    *idle_task_tcb = &idle_task_buffer;
    *idle_task_stack_buffer = idle_task_stack;
    *idle_task_stack_size = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **timer_task_tcb,
                                    StackType_t **timer_task_stack_buffer,
                                    uint32_t *timer_task_stack_size)
{
    *timer_task_tcb = &timer_task_buffer;
    *timer_task_stack_buffer = timer_task_stack;
    *timer_task_stack_size = configTIMER_TASK_STACK_DEPTH;
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *task_name)
{
    frt_stack_overflow_hook(task, task_name);
}

void vApplicationMallocFailedHook(void)
{
    frt_malloc_failed_hook();
}
