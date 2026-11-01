#ifndef MODULES_LOGGER_LOGGER_H
#define MODULES_LOGGER_LOGGER_H

#include <stdbool.h>
#include <stdint.h>

#include <modules/logger/log_format.h>
#include <modules/logger/log_storage.h>
#include <modules/logger/log_topics.h>
#include <modules/logger/log_writer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    log_writer_t writer;
    log_storage_t storage;
    log_topics_t topics;
    uint32_t dropped_records;
    uint32_t status_last_ms;
    uint64_t start_time_us;
    bool active;
} logger_t;

int logger_init(logger_t *logger, const log_backend_t *backend, uint32_t max_file_bytes);

/** Write the file header and enable record capture. */
int logger_begin(logger_t *logger, uint64_t start_time_us, uint32_t board_id, uint32_t git_hash);

/** Capture all topics whose interval has elapsed. Call at 50-100 Hz. */
int logger_update(logger_t *logger, uint32_t now_ms);

/** Flush the buffer and close the log. */
int logger_end(logger_t *logger);

int logger_start(void);
int logger_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_LOGGER_LOGGER_H */
