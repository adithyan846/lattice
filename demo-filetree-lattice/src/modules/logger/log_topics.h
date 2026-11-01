#ifndef MODULES_LOGGER_LOG_TOPICS_H
#define MODULES_LOGGER_LOG_TOPICS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_TOPIC_MAX 16
#define LOG_TOPIC_NAME_MAX 32

typedef struct {
    uint16_t id;
    char name[LOG_TOPIC_NAME_MAX];
    size_t payload_size;
    uint32_t interval_ms;
    uint32_t last_logged_ms;
    bool enabled;
} log_topic_t;

typedef struct {
    log_topic_t topics[LOG_TOPIC_MAX];
    uint8_t count;
} log_topics_t;

void log_topics_init(log_topics_t *topics);

/** Populate the default topic set from docs/logging.md. */
int log_topics_default(log_topics_t *topics);

int log_topics_add(log_topics_t *topics, const char *name, size_t payload_size,
                   uint32_t interval_ms);

log_topic_t *log_topics_find(log_topics_t *topics, const char *name);
log_topic_t *log_topics_find_id(log_topics_t *topics, uint16_t id);

/** True when the topic is enabled and its interval has elapsed. */
bool log_topics_due(const log_topic_t *topic, uint32_t now_ms);

void log_topics_mark(log_topic_t *topic, uint32_t now_ms);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_LOGGER_LOG_TOPICS_H */
