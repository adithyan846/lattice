#include <freertrtos/memory.h>

#include "FreeRTOS.h"
#include "task.h"

void *frt_malloc(size_t size)
{
    return pvPortMalloc(size);
}

void frt_free(void *ptr)
{
    vPortFree(ptr);
}

size_t frt_heap_free_bytes(void)
{
    return (size_t)xPortGetFreeHeapSize();
}

size_t frt_heap_min_free_bytes(void)
{
    return (size_t)xPortGetMinimumEverFreeHeapSize();
}
