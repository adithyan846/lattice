#ifndef MODULES_TASK_WATCHDOG_TASK_WATCHDOG_H
#define MODULES_TASK_WATCHDOG_TASK_WATCHDOG_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TASK_WATCHDOG_MAX_TASKS 16

typedef struct {
    const char *name;
    uint32_t last_checkin_ms;
    uint32_t timeout_ms;
    bool registered;
} task_watchdog_entry_t;

typedef struct {
    task_watchdog_entry_t tasks[TASK_WATCHDOG_MAX_TASKS];
    uint8_t count;
} task_watchdog_t;

void task_watchdog_init(task_watchdog_t *watchdog);

int task_watchdog_register(task_watchdog_t *watchdog, const char *name, uint32_t timeout_ms,
                           uint32_t now_ms);

int task_watchdog_checkin(task_watchdog_t *watchdog, const char *name, uint32_t now_ms);

/** Returns the number of expired tasks; logs the first offender. */
int task_watchdog_check(task_watchdog_t *watchdog, uint32_t now_ms);

int task_watchdog_start(void);
int task_watchdog_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_TASK_WATCHDOG_TASK_WATCHDOG_H */
