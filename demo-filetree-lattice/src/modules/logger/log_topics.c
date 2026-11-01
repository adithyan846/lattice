#include <modules/logger/log_topics.h>

#include <stddef.h>
#include <string.h>

#include <msg/generated/uorb_topics.h>

void log_topics_init(log_topics_t *topics)
{
    if (topics == NULL) {
        return;
    }

    memset(topics, 0, sizeof(*topics));
}

int log_topics_add(log_topics_t *topics, const char *name, size_t payload_size,
                   uint32_t interval_ms)
{
    if (topics == NULL || name == NULL || payload_size == 0 || payload_size > 256 ||
        interval_ms == 0 || topics->count >= LOG_TOPIC_MAX ||
        strlen(name) >= LOG_TOPIC_NAME_MAX) {
        return -1;
    }

    log_topic_t *topic = &topics->topics[topics->count];
    memset(topic, 0, sizeof(*topic));
    topic->id = (uint16_t)(topics->count + 1);
    strncpy(topic->name, name, LOG_TOPIC_NAME_MAX - 1);
    topic->payload_size = payload_size;
    topic->interval_ms = interval_ms;
    topic->enabled = true;
    topics->count++;
    return 0;
}

int log_topics_default(log_topics_t *topics)
{
    if (topics == NULL) {
        return -1;
    }

    log_topics_init(topics);

    const struct {
        const char *name;
        size_t size;
        uint32_t interval_ms;
    } defaults[] = {
        {"vehicle_attitude", sizeof(vehicle_attitude_s), 20},
        {"vehicle_local_position", sizeof(vehicle_local_position_s), 50},
        {"vehicle_global_position", sizeof(vehicle_global_position_s), 200},
        {"vehicle_status", sizeof(vehicle_status_s), 200},
        {"actuator_motors", sizeof(actuator_motors_s), 20},
        {"battery_status", sizeof(battery_status_s), 1000},
        {"mission_current", sizeof(mission_current_s), 500},
        {"failsafe_flags", sizeof(failsafe_flags_s), 500},
        {"estimator_status", sizeof(estimator_status_s), 500},
        {"sensor_gps", sizeof(sensor_gps_s), 200},
    };

    for (size_t index = 0; index < sizeof(defaults) / sizeof(defaults[0]); index++) {
        if (log_topics_add(topics, defaults[index].name, defaults[index].size,
                           defaults[index].interval_ms) != 0) {
            return -1;
        }
    }

    return 0;
}

log_topic_t *log_topics_find(log_topics_t *topics, const char *name)
{
    if (topics == NULL || name == NULL) {
        return NULL;
    }

    for (uint8_t index = 0; index < topics->count; index++) {
        if (strncmp(topics->topics[index].name, name, LOG_TOPIC_NAME_MAX) == 0) {
            return &topics->topics[index];
        }
    }

    return NULL;
}

log_topic_t *log_topics_find_id(log_topics_t *topics, uint16_t id)
{
    if (topics == NULL) {
        return NULL;
    }

    for (uint8_t index = 0; index < topics->count; index++) {
        if (topics->topics[index].id == id) {
            return &topics->topics[index];
        }
    }

    return NULL;
}

bool log_topics_due(const log_topic_t *topic, uint32_t now_ms)
{
    if (topic == NULL || !topic->enabled) {
        return false;
    }

    return (uint32_t)(now_ms - topic->last_logged_ms) >= topic->interval_ms;
}

void log_topics_mark(log_topic_t *topic, uint32_t now_ms)
{
    if (topic != NULL) {
        topic->last_logged_ms = now_ms;
    }
}
