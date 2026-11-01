#ifndef MODULES_MAVLINK_MAVLINK_STREAMS_H
#define MODULES_MAVLINK_MAVLINK_STREAMS_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAVLINK_STREAM_MAX 16

typedef struct {
    uint32_t message_id;
    uint32_t interval_ms;
    uint32_t last_sent_ms;
} mavlink_stream_t;

typedef struct {
    mavlink_stream_t streams[MAVLINK_STREAM_MAX];
    uint8_t count;
} mavlink_streams_t;

void mavlink_streams_init(mavlink_streams_t *streams);

/** Add or update a stream rate. Returns 0 on success, -1 if the table is full. */
int mavlink_streams_set(mavlink_streams_t *streams, uint32_t message_id, uint32_t interval_ms);

/** Configure the default GCS stream set from docs/mavlink.md. */
int mavlink_streams_default(mavlink_streams_t *streams);

/**
 * Returns true when the stream is due and records `now_ms` as its send time.
 * Unknown message IDs are never due.
 */
bool mavlink_streams_due(mavlink_streams_t *streams, uint32_t message_id, uint32_t now_ms);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_MAVLINK_MAVLINK_STREAMS_H */
