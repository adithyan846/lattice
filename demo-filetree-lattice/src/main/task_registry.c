#include <main/task_registry.h>

#include <stddef.h>
#include <string.h>

static app_task_t tasks[APP_TASK_MAX];
static size_t task_count;

void task_registry_init(void)
{
    memset(tasks, 0, sizeof(tasks));
    task_count = 0;
}

int task_registry_add(const char *name, frt_task_entry_t entry, void *argument,
                      uint32_t stack_words, uint32_t priority, uint32_t period_ms)
{
    if (name == NULL || entry == NULL || stack_words == 0 || task_count >= APP_TASK_MAX) {
        return -1;
    }

    app_task_t *task = &tasks[task_count++];
    task->name = name;
    task->entry = entry;
    task->argument = argument;
    task->stack_words = stack_words;
    task->priority = priority;
    task->period_ms = period_ms;
    task->started = 0;
    return 0;
}

int task_registry_start_all(void)
{
    int started = 0;

    for (size_t index = 0; index < task_count; index++) {
        app_task_t *task = &tasks[index];

        if (frt_task_create(task->name, task->entry, task->argument, task->stack_words,
                            task->priority, &task->handle) == FRT_OK) {
            task->started = 1;
            started++;
        }
    }

    return started;
}

const app_task_t *task_registry_find(const char *name)
{
    if (name == NULL) {
        return NULL;
    }

    for (size_t index = 0; index < task_count; index++) {
        if (strcmp(tasks[index].name, name) == 0) {
            return &tasks[index];
        }
    }

    return NULL;
}

size_t task_registry_count(void)
{
    return task_count;
}
