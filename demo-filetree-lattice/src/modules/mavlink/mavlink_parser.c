#include <modules/mavlink/mavlink_parser.h>

#include <stddef.h>
#include <string.h>

#include <lib/crc/crc16.h>

int mavlink_crc_extra(uint32_t message_id)
{
    switch (message_id) {
    case 0: /* HEARTBEAT */
        return 50;
    case 1: /* SYS_STATUS */
        return 124;
    case 2: /* SYSTEM_TIME */
        return 137;
    case 4: /* PING */
        return 237;
    case 20: /* PARAM_REQUEST_READ */
        return 214;
    case 21: /* PARAM_REQUEST_LIST */
        return 159;
    case 22: /* PARAM_VALUE */
        return 220;
    case 23: /* PARAM_SET */
        return 168;
    case 24: /* GPS_RAW_INT */
        return 24;
    case 30: /* ATTITUDE */
        return 39;
    case 32: /* LOCAL_POSITION_NED */
        return 185;
    case 33: /* GLOBAL_POSITION_INT */
        return 104;
    case 39: /* MISSION_ITEM */
        return 254;
    case 40: /* MISSION_REQUEST */
        return 230;
    case 41: /* MISSION_SET_CURRENT */
        return 28;
    case 42: /* MISSION_CURRENT */
        return 28;
    case 43: /* MISSION_REQUEST_LIST */
        return 132;
    case 44: /* MISSION_COUNT */
        return 221;
    case 45: /* MISSION_CLEAR_ALL */
        return 232;
    case 47: /* MISSION_ACK */
        return 153;
    case 51: /* MISSION_REQUEST_INT */
        return 196;
    case 73: /* MISSION_ITEM_INT */
        return 38;
    case 75: /* COMMAND_INT */
        return 158;
    case 76: /* COMMAND_LONG */
        return 152;
    case 77: /* COMMAND_ACK */
        return 143;
    case 147: /* BATTERY_STATUS */
        return 154;
    case 148: /* AUTOPILOT_VERSION */
        return 178;
    case 253: /* STATUSTEXT */
        return 83;
    default:
        return -1;
    }
}

void mavlink_parser_init(mavlink_parser_t *parser)
{
    if (parser == NULL) {
        return;
    }

    memset(parser, 0, sizeof(*parser));
}

static uint16_t read_u16(const uint8_t *data)
{
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

static uint32_t read_u24(const uint8_t *data)
{
    return (uint32_t)data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16);
}

static int header_length(const mavlink_parser_t *parser)
{
    return parser->buffer[0] == MAVLINK_V2_MAGIC ? 10 : 6;
}

static bool frame_valid(mavlink_parser_t *parser, mavlink_frame_t *frame)
{
    const uint8_t magic = parser->buffer[0];
    const uint8_t payload_length = parser->buffer[1];
    const int header = header_length(parser);
    const size_t crc_start = 1;
    const size_t crc_end = (size_t)header + payload_length;

    uint32_t message_id;

    if (magic == MAVLINK_V2_MAGIC) {
        message_id = read_u24(&parser->buffer[7]);

    } else {
        message_id = parser->buffer[5];
    }

    const int extra = mavlink_crc_extra(message_id);

    if (extra < 0) {
        return false;
    }

    uint16_t crc = crc16_accumulate(0xFFFF, &parser->buffer[crc_start], crc_end - crc_start);
    const uint8_t extra_byte = (uint8_t)extra;
    crc = crc16_accumulate(crc, &extra_byte, 1);

    if (crc != read_u16(&parser->buffer[crc_end])) {
        return false;
    }

    memset(frame, 0, sizeof(*frame));
    frame->version = magic == MAVLINK_V2_MAGIC ? 2 : 1;
    frame->sequence = parser->buffer[magic == MAVLINK_V2_MAGIC ? 4 : 2];
    frame->system_id = parser->buffer[magic == MAVLINK_V2_MAGIC ? 5 : 3];
    frame->component_id = parser->buffer[magic == MAVLINK_V2_MAGIC ? 6 : 4];
    frame->message_id = message_id;
    frame->payload_length = payload_length;
    memcpy(frame->payload, &parser->buffer[header], payload_length);
    return true;
}

static void parser_reset(mavlink_parser_t *parser)
{
    parser->length = 0;
    parser->expected = 0;
    parser->collecting = false;
}

int mavlink_parser_feed(mavlink_parser_t *parser, const uint8_t *data, size_t length,
                        mavlink_frame_callback_t callback, void *context)
{
    if (parser == NULL || (data == NULL && length > 0)) {
        return 0;
    }

    int frames = 0;

    for (size_t index = 0; index < length; index++) {
        const uint8_t byte = data[index];

        if (!parser->collecting) {
            if (byte != MAVLINK_V1_MAGIC && byte != MAVLINK_V2_MAGIC) {
                continue;
            }

            parser->collecting = true;
            parser->length = 0;
        }

        if (parser->length >= sizeof(parser->buffer)) {
            parser_reset(parser);
            continue;
        }

        parser->buffer[parser->length++] = byte;

        if (parser->length < 3) {
            continue;
        }

        const uint8_t magic = parser->buffer[0];
        const uint8_t payload_length = parser->buffer[1];
        const int header = header_length(parser);
        size_t expected = (size_t)header + payload_length + 2;

        if (magic == MAVLINK_V2_MAGIC && (parser->buffer[2] & 0x01) != 0) {
            /* Signed frame: reject rather than silently ignore authentication. */
            parser_reset(parser);
            continue;
        }

        parser->expected = expected;

        if (parser->length < expected) {
            continue;
        }

        mavlink_frame_t frame;

        if (frame_valid(parser, &frame) && callback != NULL) {
            callback(&frame, context);
            frames++;
        }

        parser_reset(parser);
    }

    return frames;
}
