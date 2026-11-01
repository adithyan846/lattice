#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*
 * FreeRTOS configuration for the reference THEJAS32 board.
 *
 * This file is part of freertrtos. It is included by the upstream FreeRTOS
 * kernel; keep it C-compatible and free of board headers.
 *
 * Values marked TODO must be verified against the actual SoC/board manual and
 * the chosen FreeRTOS port. They are placeholders, not measured numbers.
 */

#include <freertrtos/fault.h>

#define configUSE_PREEMPTION                    1
#define configUSE_TIME_SLICING                  0
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TICKLESS_IDLE                 0

#define configCPU_CLOCK_HZ                      (100000000UL) /* TODO: board clock */
#define configTICK_RATE_HZ                      ((TickType_t)1000)
#define configMAX_PRIORITIES                    (32)
#define configMINIMAL_STACK_SIZE                ((configSTACK_DEPTH_TYPE)256)
#define configMAX_TASK_NAME_LEN                 (16)

#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 0
#define configUSE_TASK_NOTIFICATIONS            1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   3
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_QUEUE_SETS                    0
#define configUSE_ALTERNATIVE_API               0
#define configQUEUE_REGISTRY_SIZE               8
#define configUSE_TRACE_FACILITY                1
#define configUSE_STATS_FORMATTING_FUNCTIONS    1

#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               (2)
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            (configMINIMAL_STACK_SIZE * 2)

#define configSUPPORT_STATIC_ALLOCATION         1
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configTOTAL_HEAP_SIZE                   ((size_t)(64 * 1024)) /* TODO: measure */
#define configAPPLICATION_ALLOCATED_HEAP        0

#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0

#define configUSE_NEWLIB_REENTRANT              0
#define configUSE_POSIX_ERRNO                   0
#define configENABLE_BACKWARD_COMPATIBILITY     0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 0

#define configUSE_EVENT_GROUPS                  1
#define configUSE_STREAM_BUFFERS                1
#define configUSE_APPLICATION_TASK_TAG          0
#define configRECORD_STACK_HIGH_ADDRESS         1
#define configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES 0

/* Enable only together with the port's FPU context-save code. */
#define configUSE_FPU                           0

#define configASSERT(x)                                                        \
    do {                                                                       \
        if ((x) == 0) {                                                        \
            frt_assert_failed(__FILE__, __LINE__);                             \
        }                                                                      \
    } while (0)

/* The RISC-V port may require additional SoC-specific defines (CLINT/MTIME,
 * interrupt controller base, etc.). Add them here after reading the port's
 * README and the THEJAS32 manual. Do not copy another board's addresses. */

#endif /* FREERTOS_CONFIG_H */
