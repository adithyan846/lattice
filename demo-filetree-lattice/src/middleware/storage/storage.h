#ifndef MIDDLEWARE_STORAGE_STORAGE_H
#define MIDDLEWARE_STORAGE_STORAGE_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Mount the flight-log filesystem (SD card or flash). */
int storage_mount(void);

bool storage_ready(void);

int storage_read_file(const char *path, void *buffer, size_t max_length, size_t *out_length);
int storage_write_file(const char *path, const void *data, size_t length);
int storage_append_file(const char *path, const void *data, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* MIDDLEWARE_STORAGE_STORAGE_H */
