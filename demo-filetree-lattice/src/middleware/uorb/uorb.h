#ifndef MIDDLEWARE_UORB_UORB_H
#define MIDDLEWARE_UORB_UORB_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UORB_MAX_TOPICS 64
#define UORB_MAX_NAME_LENGTH 32
#define UORB_MAX_MESSAGE_SIZE 256

typedef struct uorb_topic uorb_topic_t;

/**
 * Initialise the static topic registry. Call once before any module starts.
 */
int uorb_init(void);

/**
 * Look up or create a topic by name. The first caller supplies the payload
 * size; later callers with the same name get the existing topic and a size
 * mismatch is rejected.
 */
uorb_topic_t *uorb_topic(const char *name, size_t size);

/** Publish a fixed-size payload. Copies under the topic lock. */
int uorb_publish(uorb_topic_t *topic, const void *data);

/** Copy the latest payload. Returns 0 on success. */
int uorb_copy(uorb_topic_t *topic, void *data);

/**
 * Block until the topic sequence changes or the timeout expires.
 * Returns 0 if new data is available, -1 on timeout.
 */
int uorb_wait(uorb_topic_t *topic, uint32_t timeout_ms);

uint32_t uorb_sequence(uorb_topic_t *topic);

#ifdef __cplusplus
}
#endif

#endif /* MIDDLEWARE_UORB_UORB_H */
