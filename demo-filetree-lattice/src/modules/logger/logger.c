#include <modules/logger/logger.h>

#include <stddef.h>
#include <string.h>

#include <middleware/log/log.h>
#include <middleware/uorb/uorb.h>
#include <msg/generated/uorb_topics.h>

int logger_init(logger_t *logger, const log_backend_t *backend, uint32_t max_file_bytes)
{
    if (logger == NULL) {
        return -1;
    }

    memset(logger, 0, sizeof(*logger));
    log_writer_init(&logger->writer, backend);
    log_storage_init(&logger->storage, max_file_bytes);

    if (log_topics_default(&logger->topics) != 0) {
        return -1;
    }

    return 0;
}

static void publish_status(logger_t *logger, uint32_t now_ms)
{
    uorb_topic_t *topic = uorb_topic("log_status", sizeof(log_status_s));

    if (topic == NULL) {
        return;
    }

    log_status_s status = {
        .timestamp = (uint64_t)now_ms * 1000,
        .logging_active = logger->active,
        .dropped_records = logger->dropped_records,
        .bytes_written = logger->writer.bytes_written,
    };
    (void)uorb_publish(topic, &status);
}

int logger_begin(logger_t *logger, uint64_t start_time_us, uint32_t board_id, uint32_t git_hash)
{
    if (logger == NULL || logger->active) {
        return -1;
    }

    const log_header_t header = {
        .version = LOG_FORMAT_VERSION,
        .start_time_us = start_time_us,
        .board_id = board_id,
        .git_hash = git_hash,
        .parameter_block_length = 0,
    };

    uint8_t encoded[LOG_HEADER_SIZE];
    const size_t length = log_header_encode(encoded, sizeof(encoded), &header);

    if (length == 0 || log_writer_write(&logger->writer, encoded, length) != 0) {
        return -1;
    }

    logger->start_time_us = start_time_us;
    logger->active = true;
    logger->dropped_records = 0;
    return 0;
}

int logger_update(logger_t *logger, uint32_t now_ms)
{
    if (logger == NULL || !logger->active) {
        return -1;
    }

    for (uint8_t index = 0; index < logger->topics.count; index++) {
        log_topic_t *topic = &logger->topics.topics[index];

        if (!log_topics_due(topic, now_ms)) {
            continue;
        }

        uorb_topic_t *uorb = uorb_topic(topic->name, topic->payload_size);
        uint8_t payload[LOG_MAX_PAYLOAD];

        if (uorb == NULL || uorb_copy(uorb, payload) != 0) {
            continue;
        }

        const log_record_t record = {
            .topic_id = topic->id,
            .timestamp = (uint64_t)now_ms * 1000,
            .payload_length = (uint16_t)topic->payload_size,
        };

        uint8_t encoded[LOG_MAX_RECORD];
        const size_t length = log_record_encode(encoded, sizeof(encoded), &record, payload);

        if (length == 0) {
            logger->dropped_records++;
            continue;
        }

        if (log_writer_write(&logger->writer, encoded, length) != 0) {
            logger->dropped_records++;
            continue;
        }

        log_storage_note_written(&logger->storage, length);
        log_topics_mark(topic, now_ms);
    }

    if ((uint32_t)(now_ms - logger->status_last_ms) >= 1000) {
        logger->status_last_ms = now_ms;
        publish_status(logger, now_ms);
    }

    return 0;
}

int logger_end(logger_t *logger)
{
    if (logger == NULL) {
        return -1;
    }

    if (log_writer_flush(&logger->writer) != 0) {
        log_storage_mark_truncated(&logger->storage);
    }

    logger->active = false;
    return 0;
}

int logger_start(void)
{
    /* TODO: create the logger task, mount the SD card, open a log file through
     * log_storage, start logging on arm, and stop/flush on disarm. */
    FRT_LOG_WARN("logger: scaffold only, storage task not implemented");
    return -1;
}

int logger_stop(void)
{
    return 0;
}
