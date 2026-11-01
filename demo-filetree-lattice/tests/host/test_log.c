#include <modules/logger/log_format.h>

#include <stdint.h>
#include <string.h>

#include "test_assert.h"

void test_log_format(void)
{
    uint8_t buffer[LOG_MAX_RECORD];
    const log_header_t header = {
        .version = LOG_FORMAT_VERSION,
        .start_time_us = 123456789,
        .board_id = 0xA5A5,
        .git_hash = 0xDEADBEEF,
        .parameter_block_length = 0,
    };

    const size_t header_length = log_header_encode(buffer, sizeof(buffer), &header);
    TEST_ASSERT(header_length == LOG_HEADER_SIZE);

    log_header_t decoded_header;
    TEST_ASSERT(log_header_decode(buffer, header_length, &decoded_header));
    TEST_ASSERT(decoded_header.version == header.version);
    TEST_ASSERT(decoded_header.start_time_us == header.start_time_us);
    TEST_ASSERT(decoded_header.board_id == header.board_id);
    TEST_ASSERT(decoded_header.git_hash == header.git_hash);

    const uint8_t payload[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    const log_record_t record = {
        .topic_id = 7,
        .timestamp = 999999,
        .payload_length = sizeof(payload),
    };

    const size_t record_length = log_record_encode(buffer, sizeof(buffer), &record, payload);
    TEST_ASSERT(record_length == 13 + sizeof(payload) + 4);

    log_record_t decoded_record;
    const uint8_t *decoded_payload;
    size_t consumed;
    TEST_ASSERT(log_record_decode(buffer, record_length, &decoded_record, &decoded_payload,
                                  &consumed));
    TEST_ASSERT(consumed == record_length);
    TEST_ASSERT(decoded_record.topic_id == record.topic_id);
    TEST_ASSERT(decoded_record.timestamp == record.timestamp);
    TEST_ASSERT(decoded_record.payload_length == sizeof(payload));
    TEST_ASSERT(memcmp(decoded_payload, payload, sizeof(payload)) == 0);

    buffer[5] ^= 0xFF;
    TEST_ASSERT(!log_record_decode(buffer, record_length, &decoded_record, &decoded_payload,
                                   &consumed));
}
