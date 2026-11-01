#ifndef MODULES_MAVLINK_MAVLINK_PARSER_H
#define MODULES_MAVLINK_MAVLINK_PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAVLINK_V1_MAGIC 0xFE
#define MAVLINK_V2_MAGIC 0xFD
#define MAVLINK_MAX_PAYLOAD 255
#define MAVLINK_MAX_FRAME (10 + MAVLINK_MAX_PAYLOAD + 2 + 13)

typedef struct {
    uint8_t version;
    uint8_t sequence;
    uint8_t system_id;
    uint8_t component_id;
    uint32_t message_id;
    uint8_t payload[MAVLINK_MAX_PAYLOAD];
    uint8_t payload_length;
} mavlink_frame_t;

typedef void (*mavlink_frame_callback_t)(const mavlink_frame_t *frame, void *context);

typedef struct {
    uint8_t buffer[MAVLINK_MAX_FRAME];
    size_t length;
    size_t expected;
    bool collecting;
} mavlink_parser_t;

void mavlink_parser_init(mavlink_parser_t *parser);

/**
 * Feed received bytes. Returns the number of complete, valid frames delivered
 * to `callback`. Signed MAVLink 2 frames are rejected until a signing key store
 * is implemented.
 */
int mavlink_parser_feed(mavlink_parser_t *parser, const uint8_t *data, size_t length,
                        mavlink_frame_callback_t callback, void *context);

/** Return the MAVLink CRC extra byte for a message ID, or -1 if unknown. */
int mavlink_crc_extra(uint32_t message_id);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_MAVLINK_MAVLINK_PARSER_H */
