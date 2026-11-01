#include <modules/logger/log_format.h>

#include <stddef.h>
#include <string.h>

#include <lib/crc/crc32.h>

static void put_u16(uint8_t *buffer, uint16_t value)
{
    buffer[0] = (uint8_t)(value & 0xFF);
    buffer[1] = (uint8_t)(value >> 8);
}

static void put_u32(uint8_t *buffer, uint32_t value)
{
    buffer[0] = (uint8_t)(value & 0xFF);
    buffer[1] = (uint8_t)((value >> 8) & 0xFF);
    buffer[2] = (uint8_t)((value >> 16) & 0xFF);
    buffer[3] = (uint8_t)((value >> 24) & 0xFF);
}

static void put_u64(uint8_t *buffer, uint64_t value)
{
    for (int index = 0; index < 8; index++) {
        buffer[index] = (uint8_t)((value >> (index * 8)) & 0xFF);
    }
}

static uint16_t get_u16(const uint8_t *buffer)
{
    return (uint16_t)buffer[0] | ((uint16_t)buffer[1] << 8);
}

static uint32_t get_u32(const uint8_t *buffer)
{
    return (uint32_t)buffer[0] | ((uint32_t)buffer[1] << 8) | ((uint32_t)buffer[2] << 16) |
           ((uint32_t)buffer[3] << 24);
}

static uint64_t get_u64(const uint8_t *buffer)
{
    uint64_t value = 0;

    for (int index = 0; index < 8; index++) {
        value |= (uint64_t)buffer[index] << (index * 8);
    }

    return value;
}

size_t log_header_encode(uint8_t *buffer, size_t capacity, const log_header_t *header)
{
    if (buffer == NULL || header == NULL || capacity < LOG_HEADER_SIZE) {
        return 0;
    }

    buffer[0] = 'F';
    buffer[1] = 'R';
    buffer[2] = 'T';
    buffer[3] = 'L';
    put_u16(&buffer[4], header->version);
    put_u64(&buffer[6], header->start_time_us);
    put_u32(&buffer[14], header->board_id);
    put_u32(&buffer[18], header->git_hash);
    put_u32(&buffer[22], header->parameter_block_length);
    return LOG_HEADER_SIZE;
}

bool log_header_decode(const uint8_t *buffer, size_t length, log_header_t *header)
{
    if (buffer == NULL || header == NULL || length < LOG_HEADER_SIZE) {
        return false;
    }

    if (buffer[0] != 'F' || buffer[1] != 'R' || buffer[2] != 'T' || buffer[3] != 'L') {
        return false;
    }

    header->version = get_u16(&buffer[4]);
    header->start_time_us = get_u64(&buffer[6]);
    header->board_id = get_u32(&buffer[14]);
    header->git_hash = get_u32(&buffer[18]);
    header->parameter_block_length = get_u32(&buffer[22]);
    return header->version == LOG_FORMAT_VERSION;
}

size_t log_record_encode(uint8_t *buffer, size_t capacity, const log_record_t *record,
                         const uint8_t *payload)
{
    if (buffer == NULL || record == NULL || record->payload_length > LOG_MAX_PAYLOAD ||
        capacity < (size_t)record->payload_length + 17) {
        return 0;
    }

    if (payload == NULL && record->payload_length > 0) {
        return 0;
    }

    buffer[0] = LOG_RECORD_MAGIC;
    put_u16(&buffer[1], record->topic_id);
    put_u64(&buffer[3], record->timestamp);
    put_u16(&buffer[11], record->payload_length);

    if (record->payload_length > 0) {
        memcpy(&buffer[13], payload, record->payload_length);
    }

    const size_t crc_offset = 13 + record->payload_length;
    const uint32_t crc = crc32_calculate(buffer, crc_offset);
    put_u32(&buffer[crc_offset], crc);
    return crc_offset + 4;
}

bool log_record_decode(const uint8_t *buffer, size_t length, log_record_t *record,
                       const uint8_t **payload, size_t *consumed)
{
    if (buffer == NULL || record == NULL || payload == NULL || consumed == NULL || length < 17 ||
        buffer[0] != LOG_RECORD_MAGIC) {
        return false;
    }

    const uint16_t payload_length = get_u16(&buffer[11]);

    if (payload_length > LOG_MAX_PAYLOAD) {
        return false;
    }

    const size_t record_size = 13 + payload_length + 4;

    if (length < record_size) {
        return false;
    }

    const uint32_t expected_crc = crc32_calculate(buffer, 13 + payload_length);
    const uint32_t stored_crc = get_u32(&buffer[13 + payload_length]);

    if (expected_crc != stored_crc) {
        return false;
    }

    record->topic_id = get_u16(&buffer[1]);
    record->timestamp = get_u64(&buffer[3]);
    record->payload_length = payload_length;
    *payload = &buffer[13];
    *consumed = record_size;
    return true;
}
