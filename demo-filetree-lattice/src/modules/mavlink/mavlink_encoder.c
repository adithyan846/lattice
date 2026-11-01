#include <modules/mavlink/mavlink_encoder.h>

#include <stddef.h>
#include <string.h>

#include <lib/crc/crc16.h>
#include <modules/mavlink/mavlink_parser.h>

void mavlink_writer_init(mavlink_writer_t *writer, uint8_t *data, size_t capacity)
{
    if (writer == NULL) {
        return;
    }

    writer->data = data;
    writer->capacity = capacity;
    writer->length = 0;
    writer->overflow = data == NULL;
}

static void put_byte(mavlink_writer_t *writer, uint8_t value)
{
    if (writer->overflow) {
        return;
    }

    if (writer->length >= writer->capacity) {
        writer->overflow = true;
        return;
    }

    writer->data[writer->length++] = value;
}

void mavlink_writer_u8(mavlink_writer_t *writer, uint8_t value)
{
    if (writer != NULL) {
        put_byte(writer, value);
    }
}

void mavlink_writer_u16(mavlink_writer_t *writer, uint16_t value)
{
    if (writer == NULL) {
        return;
    }

    put_byte(writer, (uint8_t)(value & 0xFF));
    put_byte(writer, (uint8_t)(value >> 8));
}

void mavlink_writer_u32(mavlink_writer_t *writer, uint32_t value)
{
    if (writer == NULL) {
        return;
    }

    put_byte(writer, (uint8_t)(value & 0xFF));
    put_byte(writer, (uint8_t)((value >> 8) & 0xFF));
    put_byte(writer, (uint8_t)((value >> 16) & 0xFF));
    put_byte(writer, (uint8_t)((value >> 24) & 0xFF));
}

void mavlink_writer_u64(mavlink_writer_t *writer, uint64_t value)
{
    if (writer == NULL) {
        return;
    }

    for (int shift = 0; shift < 64; shift += 8) {
        put_byte(writer, (uint8_t)((value >> shift) & 0xFF));
    }
}

void mavlink_writer_i16(mavlink_writer_t *writer, int16_t value)
{
    mavlink_writer_u16(writer, (uint16_t)value);
}

void mavlink_writer_i32(mavlink_writer_t *writer, int32_t value)
{
    mavlink_writer_u32(writer, (uint32_t)value);
}

void mavlink_writer_f32(mavlink_writer_t *writer, float value)
{
    uint32_t bits;
    memcpy(&bits, &value, sizeof(bits));
    mavlink_writer_u32(writer, bits);
}

void mavlink_writer_bytes(mavlink_writer_t *writer, const uint8_t *data, size_t length)
{
    if (writer == NULL || (data == NULL && length > 0)) {
        return;
    }

    for (size_t index = 0; index < length; index++) {
        put_byte(writer, data[index]);
    }
}

void mavlink_reader_init(mavlink_reader_t *reader, const uint8_t *data, size_t length)
{
    if (reader == NULL) {
        return;
    }

    reader->data = data;
    reader->length = length;
    reader->offset = 0;
}

static bool reader_take(mavlink_reader_t *reader, uint8_t *value)
{
    if (reader == NULL || reader->data == NULL || reader->offset >= reader->length) {
        return false;
    }

    *value = reader->data[reader->offset++];
    return true;
}

uint8_t mavlink_reader_u8(mavlink_reader_t *reader)
{
    uint8_t value = 0;
    (void)reader_take(reader, &value);
    return value;
}

uint16_t mavlink_reader_u16(mavlink_reader_t *reader)
{
    uint8_t low = 0;
    uint8_t high = 0;
    (void)reader_take(reader, &low);
    (void)reader_take(reader, &high);
    return (uint16_t)low | ((uint16_t)high << 8);
}

uint32_t mavlink_reader_u32(mavlink_reader_t *reader)
{
    const uint16_t low = mavlink_reader_u16(reader);
    const uint16_t high = mavlink_reader_u16(reader);
    return (uint32_t)low | ((uint32_t)high << 16);
}

uint64_t mavlink_reader_u64(mavlink_reader_t *reader)
{
    const uint32_t low = mavlink_reader_u32(reader);
    const uint32_t high = mavlink_reader_u32(reader);
    return (uint64_t)low | ((uint64_t)high << 32);
}

int16_t mavlink_reader_i16(mavlink_reader_t *reader)
{
    return (int16_t)mavlink_reader_u16(reader);
}

int32_t mavlink_reader_i32(mavlink_reader_t *reader)
{
    return (int32_t)mavlink_reader_u32(reader);
}

float mavlink_reader_f32(mavlink_reader_t *reader)
{
    const uint32_t bits = mavlink_reader_u32(reader);
    float value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

bool mavlink_reader_ok(const mavlink_reader_t *reader)
{
    return reader != NULL && reader->data != NULL && !(reader->offset > reader->length);
}

int mavlink_encode_v2(uint8_t sequence, uint8_t system_id, uint8_t component_id,
                      uint32_t message_id, const uint8_t *payload, uint8_t payload_length,
                      uint8_t *buffer, size_t capacity, size_t *written)
{
    if (buffer == NULL || written == NULL || message_id > 0xFFFFFF ||
        (payload == NULL && payload_length > 0)) {
        return -1;
    }

    const int extra = mavlink_crc_extra(message_id);

    if (extra < 0) {
        return -1;
    }

    const size_t frame_length = 10 + payload_length + 2;

    if (capacity < frame_length) {
        return -1;
    }

    buffer[0] = MAVLINK_V2_MAGIC;
    buffer[1] = payload_length;
    buffer[2] = 0; /* incompatibility flags */
    buffer[3] = 0; /* compatibility flags */
    buffer[4] = sequence;
    buffer[5] = system_id;
    buffer[6] = component_id;
    buffer[7] = (uint8_t)(message_id & 0xFF);
    buffer[8] = (uint8_t)((message_id >> 8) & 0xFF);
    buffer[9] = (uint8_t)((message_id >> 16) & 0xFF);

    if (payload_length > 0) {
        memcpy(&buffer[10], payload, payload_length);
    }

    uint16_t crc = crc16_accumulate(0xFFFF, &buffer[1], 9 + payload_length);
    const uint8_t extra_byte = (uint8_t)extra;
    crc = crc16_accumulate(crc, &extra_byte, 1);

    buffer[10 + payload_length] = (uint8_t)(crc & 0xFF);
    buffer[11 + payload_length] = (uint8_t)(crc >> 8);

    *written = frame_length;
    return 0;
}
