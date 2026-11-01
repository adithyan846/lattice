#include <modules/task_watchdog/task_watchdog.h>

#include <stddef.h>
#include <string.h>

#include <middleware/log/log.h>

void task_watchdog_init(task_watchdog_t *watchdog)
{
    if (watchdog != NULL) {
        memset(watchdog, 0, sizeof(*watchdog));
    }
}

int task_watchdog_register(task_watchdog_t *watchdog, const char *name, uint32_t timeout_ms,
                           uint32_t now_ms)
{
    if (watchdog == NULL || name == NULL || timeout_ms == 0) {
        return -1;
    }

    for (uint8_t index = 0; index < watchdog->count; index++) {
        if (watchdog->tasks[index].registered && strcmp(watchdog->tasks[index].name, name) == 0) {
            watchdog->tasks[index].timeout_ms = timeout_ms;
            watchdog->tasks[index].last_checkin_ms = now_ms;
            return 0;
        }
    }

    if (watchdog->count >= TASK_WATCHDOG_MAX_TASKS) {
        return -1;
    }

    task_watchdog_entry_t *entry = &watchdog->tasks[watchdog->count++];
    entry->name = name;
    entry->timeout_ms = timeout_ms;
    entry->last_checkin_ms = now_ms;
    entry->registered = true;
    return 0;
}

int task_watchdog_checkin(task_watchdog_t *watchdog, const char *name, uint32_t now_ms)
{
    if (watchdog == NULL || name == NULL) {
        return -1;
    }

    for (uint8_t index = 0; index < watchdog->count; index++) {
        if (watchdog->tasks[index].registered && strcmp(watchdog->tasks[index].name, name) == 0) {
            watchdog->tasks[index].last_checkin_ms = now_ms;
            return 0;
        }
    }

    return -1;
}

int task_watchdog_check(task_watchdog_t *watchdog, uint32_t now_ms)
{
    if (watchdog == NULL) {
        return -1;
    }

    int expired = 0;

    for (uint8_t index = 0; index < watchdog->count; index++) {
        const task_watchdog_entry_t *entry = &watchdog->tasks[index];

        if (!entry->registered) {
            continue;
        }

        if ((uint32_t)(now_ms - entry->last_checkin_ms) > entry->timeout_ms) {
            if (expired == 0) {
                FRT_LOG_ERROR("task_watchdog: %s missed its deadline", entry->name);
            }

            expired++;
        }
    }

    return expired;
}

int task_watchdog_start(void)
{
    /* TODO: create the watchdog task and register every periodic task. */
    FRT_LOG_WARN("task_watchdog: scaffold only, watchdog task not implemented");
    return -1;
}

int task_watchdog_stop(void)
{
    return 0;
}
