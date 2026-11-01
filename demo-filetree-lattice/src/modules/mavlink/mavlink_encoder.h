#ifndef MODULES_MAVLINK_MAVLINK_ENCODER_H
#define MODULES_MAVLINK_MAVLINK_ENCODER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Bounds-checked little-endian payload writer. */
typedef struct {
    uint8_t *data;
    size_t capacity;
    size_t length;
    bool overflow;
} mavlink_writer_t;

void mavlink_writer_init(mavlink_writer_t *writer, uint8_t *data, size_t capacity);
void mavlink_writer_u8(mavlink_writer_t *writer, uint8_t value);
void mavlink_writer_u16(mavlink_writer_t *writer, uint16_t value);
void mavlink_writer_u32(mavlink_writer_t *writer, uint32_t value);
void mavlink_writer_u64(mavlink_writer_t *writer, uint64_t value);
void mavlink_writer_i16(mavlink_writer_t *writer, int16_t value);
void mavlink_writer_i32(mavlink_writer_t *writer, int32_t value);
void mavlink_writer_f32(mavlink_writer_t *writer, float value);
void mavlink_writer_bytes(mavlink_writer_t *writer, const uint8_t *data, size_t length);

typedef struct {
    const uint8_t *data;
    size_t length;
    size_t offset;
} mavlink_reader_t;

void mavlink_reader_init(mavlink_reader_t *reader, const uint8_t *data, size_t length);
uint8_t mavlink_reader_u8(mavlink_reader_t *reader);
uint16_t mavlink_reader_u16(mavlink_reader_t *reader);
uint32_t mavlink_reader_u32(mavlink_reader_t *reader);
uint64_t mavlink_reader_u64(mavlink_reader_t *reader);
int16_t mavlink_reader_i16(mavlink_reader_t *reader);
int32_t mavlink_reader_i32(mavlink_reader_t *reader);
float mavlink_reader_f32(mavlink_reader_t *reader);
bool mavlink_reader_ok(const mavlink_reader_t *reader);

/**
 * Build a MAVLink 2 frame. Returns 0 on success and writes the frame length to
 * `written`. Returns -1 if the buffer is too small or the message ID has no
 * known CRC extra.
 */
int mavlink_encode_v2(uint8_t sequence, uint8_t system_id, uint8_t component_id,
                      uint32_t message_id, const uint8_t *payload, uint8_t payload_length,
                      uint8_t *buffer, size_t capacity, size_t *written);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_MAVLINK_MAVLINK_ENCODER_H */
