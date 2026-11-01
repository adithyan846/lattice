#ifndef MAIN_TASK_REGISTRY_H
#define MAIN_TASK_REGISTRY_H

#include <stddef.h>
#include <stdint.h>

#include <freertrtos/task.h>

#ifdef __cplusplus
extern "C" {
#endif

#define APP_TASK_MAX 16

typedef struct {
    const char *name;
    frt_task_entry_t entry;
    void *argument;
    uint32_t stack_words;
    uint32_t priority;
    uint32_t period_ms;
    frt_task_t handle;
    int started;
} app_task_t;

void task_registry_init(void);

int task_registry_add(const char *name, frt_task_entry_t entry, void *argument,
                      uint32_t stack_words, uint32_t priority, uint32_t period_ms);

/** Create every registered task. Returns the number started. */
int task_registry_start_all(void);

const app_task_t *task_registry_find(const char *name);
size_t task_registry_count(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_TASK_REGISTRY_H */
