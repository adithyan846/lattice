#include <middleware/uorb/uorb.h>

#include <stdbool.h>
#include <string.h>

#include <freertrtos/semphr.h>
#include <freertrtos/task.h>
#include <freertrtos/time.h>

struct uorb_topic {
    char name[UORB_MAX_NAME_LENGTH];
    size_t size;
    uint32_t sequence;
    uint8_t data[UORB_MAX_MESSAGE_SIZE];
    frt_mutex_t lock;
    bool used;
};

static struct uorb_topic topics[UORB_MAX_TOPICS];
static frt_mutex_t registry_lock;
static bool initialized;

int uorb_init(void)
{
    memset(topics, 0, sizeof(topics));
    registry_lock = frt_mutex_create();
    initialized = (registry_lock != NULL);
    return initialized ? 0 : -1;
}

uorb_topic_t *uorb_topic(const char *name, size_t size)
{
    if (!initialized || name == NULL || size == 0 || size > UORB_MAX_MESSAGE_SIZE) {
        return NULL;
    }

    if (frt_mutex_take(registry_lock, 100) != pdTRUE) {
        return NULL;
    }

    for (int i = 0; i < UORB_MAX_TOPICS; i++) {
        if (topics[i].used && strncmp(topics[i].name, name, UORB_MAX_NAME_LENGTH) == 0) {
            const bool size_matches = (topics[i].size == size);
            (void)frt_mutex_give(registry_lock);
            return size_matches ? &topics[i] : NULL;
        }
    }

    for (int i = 0; i < UORB_MAX_TOPICS; i++) {
        if (!topics[i].used) {
            topics[i].lock = frt_mutex_create();

            if (topics[i].lock == NULL) {
                break;
            }

            strncpy(topics[i].name, name, UORB_MAX_NAME_LENGTH - 1);
            topics[i].name[UORB_MAX_NAME_LENGTH - 1] = '\0';
            topics[i].size = size;
            topics[i].sequence = 0;
            topics[i].used = true;
            (void)frt_mutex_give(registry_lock);
            return &topics[i];
        }
    }

    (void)frt_mutex_give(registry_lock);
    return NULL;
}

int uorb_publish(uorb_topic_t *topic, const void *data)
{
    if (topic == NULL || data == NULL) {
        return -1;
    }

    if (frt_mutex_take(topic->lock, 10) != pdTRUE) {
        return -1;
    }

    memcpy(topic->data, data, topic->size);
    topic->sequence++;
    (void)frt_mutex_give(topic->lock);
    return 0;
}

int uorb_copy(uorb_topic_t *topic, void *data)
{
    if (topic == NULL || data == NULL) {
        return -1;
    }

    if (frt_mutex_take(topic->lock, 10) != pdTRUE) {
        return -1;
    }

    memcpy(data, topic->data, topic->size);
    (void)frt_mutex_give(topic->lock);
    return 0;
}

uint32_t uorb_sequence(uorb_topic_t *topic)
{
    if (topic == NULL) {
        return 0;
    }

    if (frt_mutex_take(topic->lock, 10) != pdTRUE) {
        return 0;
    }

    const uint32_t sequence = topic->sequence;
    (void)frt_mutex_give(topic->lock);
    return sequence;
}

int uorb_wait(uorb_topic_t *topic, uint32_t timeout_ms)
{
    if (topic == NULL) {
        return -1;
    }

    const uint32_t start_sequence = uorb_sequence(topic);
    const uint64_t start_time = frt_time_ms();

    for (;;) {
        if (uorb_sequence(topic) != start_sequence) {
            return 0;
        }

        if ((frt_time_ms() - start_time) >= timeout_ms) {
            return -1;
        }

        frt_delay_ms(1);
    }
}
