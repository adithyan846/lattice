#ifndef MODULES_LOGGER_LOG_WRITER_H
#define MODULES_LOGGER_LOG_WRITER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_WRITER_BUFFER_SIZE 512

typedef struct {
    void *context;
    int (*write)(void *context, const uint8_t *data, size_t length);
    int (*flush)(void *context);
} log_backend_t;

typedef struct {
    log_backend_t backend;
    uint8_t buffer[LOG_WRITER_BUFFER_SIZE];
    size_t used;
    uint32_t bytes_written;
    uint32_t write_errors;
} log_writer_t;

void log_writer_init(log_writer_t *writer, const log_backend_t *backend);

/** Buffer bytes, flushing when full. Returns 0 on success. */
int log_writer_write(log_writer_t *writer, const uint8_t *data, size_t length);

/** Flush and call the backend's flush hook. */
int log_writer_flush(log_writer_t *writer);

/** Discard buffered bytes (used after a failed storage backend). */
void log_writer_reset(log_writer_t *writer);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_LOGGER_LOG_WRITER_H */
