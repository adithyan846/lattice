#include <modules/mavlink/mavlink_commands.h>
#include <modules/mavlink/mavlink_encoder.h>
#include <modules/mavlink/mavlink_messages.h>
#include <modules/mavlink/mavlink_mission.h>
#include <modules/mavlink/mavlink_parser.h>

#include <stdint.h>

#include "test_assert.h"

static int g_frames;
static mavlink_frame_t g_last_frame;

static void frame_handler(const mavlink_frame_t *frame, void *context)
{
    (void)context;
    g_frames++;
    g_last_frame = *frame;
}

void test_mavlink_framing(void)
{
    const mavlink_heartbeat_t heartbeat = {
        .custom_mode = 2,
        .type = MAV_TYPE_QUADROTOR,
        .autopilot = MAV_AUTOPILOT_GENERIC,
        .base_mode = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED,
        .system_status = MAV_STATE_STANDBY,
        .mavlink_version = 3,
    };

    uint8_t payload[16];
    const size_t payload_length = mavlink_pack_heartbeat(payload, sizeof(payload), &heartbeat);
    TEST_ASSERT(payload_length == 9);

    uint8_t frame[64];
    size_t frame_length;
    TEST_ASSERT(mavlink_encode_v2(1, 1, 1, MAVLINK_MSG_ID_HEARTBEAT, payload,
                                  (uint8_t)payload_length, frame, sizeof(frame),
                                  &frame_length) == 0);
    TEST_ASSERT(frame_length == 10 + payload_length + 2);

    mavlink_parser_t parser;
    mavlink_parser_init(&parser);
    g_frames = 0;
    TEST_ASSERT(mavlink_parser_feed(&parser, frame, frame_length, frame_handler, NULL) == 1);
    TEST_ASSERT(g_frames == 1);
    TEST_ASSERT(g_last_frame.message_id == MAVLINK_MSG_ID_HEARTBEAT);
    TEST_ASSERT(g_last_frame.system_id == 1);

    mavlink_heartbeat_t decoded;
    TEST_ASSERT(mavlink_unpack_heartbeat(g_last_frame.payload, g_last_frame.payload_length,
                                         &decoded));
    TEST_ASSERT(decoded.custom_mode == heartbeat.custom_mode);
    TEST_ASSERT(decoded.base_mode == heartbeat.base_mode);

    frame[12] ^= 0xFF;
    TEST_ASSERT(mavlink_parser_feed(&parser, frame, frame_length, frame_handler, NULL) == 0);
}

void test_mavlink_command_mapping(void)
{
    mavlink_command_long_t command = {0};
    mavlink_command_t mapped;

    command.command = MAV_CMD_COMPONENT_ARM_DISARM;
    command.param1 = 1.0f;
    TEST_ASSERT(mavlink_command_map(&command, &mapped) == MAV_RESULT_ACCEPTED);
    TEST_ASSERT(mapped.action == MAVLINK_ACTION_ARM);

    command.command = MAV_CMD_DO_SET_MODE;
    command.param2 = 5.0f; /* custom mode 5 = mission */
    TEST_ASSERT(mavlink_command_map(&command, &mapped) == MAV_RESULT_ACCEPTED);
    TEST_ASSERT(mapped.action == MAVLINK_ACTION_SET_MODE);
    TEST_ASSERT(mapped.mode == FLIGHT_MODE_MISSION);

    command.command = 9999;
    TEST_ASSERT(mavlink_command_map(&command, &mapped) == MAV_RESULT_UNSUPPORTED);
}

void test_mavlink_mission(void)
{
    mavlink_mission_upload_t upload;
    mavlink_mission_upload_init(&upload);
    TEST_ASSERT(mavlink_mission_upload_begin(&upload, 2) == 0);

    mavlink_mission_item_int_t item = {
        .target_system = 1,
        .target_component = 1,
        .seq = 0,
        .frame = 6,
        .command = MAV_CMD_NAV_WAYPOINT,
        .autocontinue = 1,
        .param2 = 2.0f,
        .x = 473977419,
        .y = 85455938,
        .z = 20.0f,
    };
    TEST_ASSERT(mavlink_mission_upload_item(&upload, &item) == 0);

    item.seq = 1;
    item.command = MAV_CMD_NAV_RETURN_TO_LAUNCH;
    TEST_ASSERT(mavlink_mission_upload_item(&upload, &item) == 0);
    TEST_ASSERT(mavlink_mission_upload_complete(&upload));
    TEST_ASSERT(mavlink_mission_upload_finish(&upload) == MAV_MISSION_ACCEPTED);
    TEST_ASSERT(mavlink_mission_upload_mission(&upload)->count == 2);
}
