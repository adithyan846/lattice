#ifndef MODULES_LOGGER_LOG_STORAGE_H
#define MODULES_LOGGER_LOG_STORAGE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t max_file_bytes;
    uint32_t file_bytes;
    uint32_t file_index;
    bool truncated;
} log_storage_t;

void log_storage_init(log_storage_t *storage, uint32_t max_file_bytes);

/**
 * Build `/fs/microsd/log/YYYY-MM-DD/HH-MM-SS_freertrtos.ftlog` style paths.
 * `root` is normally `/fs/microsd/log`. Returns 0 on success.
 */
int log_storage_path(char *buffer, size_t capacity, const char *root, const char *date,
                     const char *time, uint32_t index);

bool log_storage_should_rotate(const log_storage_t *storage, size_t incoming_bytes);

void log_storage_note_written(log_storage_t *storage, size_t bytes);

void log_storage_begin_file(log_storage_t *storage);

void log_storage_mark_truncated(log_storage_t *storage);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_LOGGER_LOG_STORAGE_H */
