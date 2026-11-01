#ifndef FREERTRTOS_MEMORY_H
#define FREERTRTOS_MEMORY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Kernel heap. Do not use in a periodic flight path. */
void *frt_malloc(size_t size);
void frt_free(void *ptr);
size_t frt_heap_free_bytes(void);
size_t frt_heap_min_free_bytes(void);

#ifdef __cplusplus
}
#endif

#endif /* FREERTRTOS_MEMORY_H */
