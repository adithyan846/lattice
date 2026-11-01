#ifndef MODULES_LOGGER_LOG_FORMAT_H
#define MODULES_LOGGER_LOG_FORMAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_FORMAT_VERSION 1
#define LOG_RECORD_MAGIC 0xA5
#define LOG_MAX_PAYLOAD 256
#define LOG_MAX_RECORD (1 + 2 + 8 + 2 + LOG_MAX_PAYLOAD + 4)
#define LOG_HEADER_SIZE 26

typedef struct {
    uint16_t version;
    uint64_t start_time_us;
    uint32_t board_id;
    uint32_t git_hash;
    uint32_t parameter_block_length;
} log_header_t;

typedef struct {
    uint16_t topic_id;
    uint64_t timestamp;
    uint16_t payload_length;
} log_record_t;

/** Encode the "FRTL" header. Returns bytes written, or 0 on error. */
size_t log_header_encode(uint8_t *buffer, size_t capacity, const log_header_t *header);

/** Decode and validate the header magic. Returns true on success. */
bool log_header_decode(const uint8_t *buffer, size_t length, log_header_t *header);

/** Encode one record including its CRC32. Returns bytes written, or 0. */
size_t log_record_encode(uint8_t *buffer, size_t capacity, const log_record_t *record,
                         const uint8_t *payload);

/**
 * Decode the next record from `buffer`. On success `payload` points into
 * `buffer` and `consumed` is the record size.
 */
bool log_record_decode(const uint8_t *buffer, size_t length, log_record_t *record,
                       const uint8_t **payload, size_t *consumed);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_LOGGER_LOG_FORMAT_H */
