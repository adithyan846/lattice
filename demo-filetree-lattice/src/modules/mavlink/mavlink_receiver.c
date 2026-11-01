#include <modules/mavlink/mavlink_receiver.h>

#include <stddef.h>

#include <modules/mavlink/mavlink_messages.h>

static bool addressed_to_us(const mavlink_receiver_t *receiver, uint8_t target_system,
                            uint8_t target_component)
{
    const bool system_matches = target_system == 0 || target_system == receiver->system_id;
    const bool component_matches =
        target_component == 0 || target_component == receiver->component_id;
    return system_matches && component_matches;
}

static void handle_command(mavlink_receiver_t *receiver, const mavlink_frame_t *frame)
{
    mavlink_command_long_t command;

    if (!mavlink_unpack_command_long(frame->payload, frame->payload_length, &command)) {
        return;
    }

    if (!addressed_to_us(receiver, command.target_system, command.target_component)) {
        return;
    }

    mavlink_command_t mapped;

    if (mavlink_command_map(&command, &mapped) == MAV_RESULT_ACCEPTED &&
        mapped.action != MAVLINK_ACTION_NONE && receiver->callbacks.command != NULL) {
        receiver->callbacks.command(&mapped, receiver->callbacks.context);
    }
}

static void handle_mission_count(mavlink_receiver_t *receiver, const mavlink_frame_t *frame)
{
    mavlink_mission_count_t count;

    if (!mavlink_unpack_mission_count(frame->payload, frame->payload_length, &count)) {
        return;
    }

    if (!addressed_to_us(receiver, count.target_system, count.target_component)) {
        return;
    }

    if (mavlink_mission_upload_begin(&receiver->upload, count.count) != 0) {
        return;
    }

    if (receiver->callbacks.mission_request != NULL) {
        receiver->callbacks.mission_request(0, receiver->callbacks.context);
    }
}

static void handle_mission_item(mavlink_receiver_t *receiver, const mavlink_frame_t *frame)
{
    mavlink_mission_item_int_t item;

    if (!mavlink_unpack_mission_item_int(frame->payload, frame->payload_length, &item)) {
        return;
    }

    if (!addressed_to_us(receiver, item.target_system, item.target_component)) {
        return;
    }

    if (mavlink_mission_upload_item(&receiver->upload, &item) != 0) {
        return;
    }

    if (mavlink_mission_upload_complete(&receiver->upload)) {
        const uint8_t result = mavlink_mission_upload_finish(&receiver->upload);

        if (receiver->callbacks.mission_upload_finished != NULL) {
            receiver->callbacks.mission_upload_finished(
                result, mavlink_mission_upload_mission(&receiver->upload),
                receiver->callbacks.context);
        }

        return;
    }

    if (receiver->callbacks.mission_request != NULL) {
        receiver->callbacks.mission_request(item.seq + 1, receiver->callbacks.context);
    }
}

static void handle_mission_request(mavlink_receiver_t *receiver, const mavlink_frame_t *frame)
{
    mavlink_mission_request_t request;

    if (!mavlink_unpack_mission_request(frame->payload, frame->payload_length, &request)) {
        return;
    }

    if (!addressed_to_us(receiver, request.target_system, request.target_component)) {
        return;
    }

    if (receiver->callbacks.mission_item_requested != NULL) {
        receiver->callbacks.mission_item_requested(request.seq, receiver->callbacks.context);
    }
}

static void handle_param_set(mavlink_receiver_t *receiver, const mavlink_frame_t *frame)
{
    mavlink_param_set_t parameter;

    if (!mavlink_unpack_param_set(frame->payload, frame->payload_length, &parameter)) {
        return;
    }

    if (!addressed_to_us(receiver, parameter.target_system, parameter.target_component)) {
        return;
    }

    if (receiver->callbacks.param_set != NULL) {
        receiver->callbacks.param_set(parameter.param_id, parameter.param_value, parameter.param_type,
                                      receiver->callbacks.context);
    }
}

static void frame_callback(const mavlink_frame_t *frame, void *context)
{
    mavlink_receiver_t *receiver = context;

    if (receiver == NULL || frame == NULL) {
        return;
    }

    switch (frame->message_id) {
    case MAVLINK_MSG_ID_HEARTBEAT:
        receiver->last_heartbeat_ms = receiver->now_ms;
        receiver->heartbeat_received = true;
        receiver->link_connected = true;
        break;

    case MAVLINK_MSG_ID_COMMAND_LONG:
        handle_command(receiver, frame);
        break;

    case MAVLINK_MSG_ID_MISSION_COUNT:
        handle_mission_count(receiver, frame);
        break;

    case MAVLINK_MSG_ID_MISSION_ITEM_INT:
        handle_mission_item(receiver, frame);
        break;

    case MAVLINK_MSG_ID_MISSION_REQUEST_INT:
    case 40: /* MISSION_REQUEST (legacy float variant, same routing fields) */
        handle_mission_request(receiver, frame);
        break;

    case MAVLINK_MSG_ID_MISSION_REQUEST_LIST:
        if (receiver->callbacks.mission_download_requested != NULL) {
            receiver->callbacks.mission_download_requested(receiver->callbacks.context);
        }

        break;

    case MAVLINK_MSG_ID_MISSION_CLEAR_ALL:
        mavlink_mission_upload_clear(&receiver->upload);

        if (receiver->callbacks.mission_cleared != NULL) {
            receiver->callbacks.mission_cleared(receiver->callbacks.context);
        }

        break;

    case MAVLINK_MSG_ID_MISSION_SET_CURRENT:
        if (frame->payload_length >= 4) {
            const uint16_t seq = (uint16_t)(frame->payload[2] | (frame->payload[3] << 8));

            if (receiver->callbacks.mission_set_current != NULL) {
                receiver->callbacks.mission_set_current(seq, receiver->callbacks.context);
            }
        }

        break;

    case MAVLINK_MSG_ID_PARAM_SET:
        handle_param_set(receiver, frame);
        break;

    default:
        break;
    }
}

void mavlink_receiver_init(mavlink_receiver_t *receiver, uint8_t system_id, uint8_t component_id)
{
    if (receiver == NULL) {
        return;
    }

    mavlink_parser_init(&receiver->parser);
    mavlink_mission_upload_init(&receiver->upload);
    receiver->system_id = system_id;
    receiver->component_id = component_id;
    receiver->last_heartbeat_ms = 0;
    receiver->now_ms = 0;
    receiver->heartbeat_received = false;
    receiver->link_connected = false;
    receiver->callbacks = (mavlink_receiver_callbacks_t){0};
}

void mavlink_receiver_set_callbacks(mavlink_receiver_t *receiver,
                                    const mavlink_receiver_callbacks_t *callbacks)
{
    if (receiver == NULL || callbacks == NULL) {
        return;
    }

    receiver->callbacks = *callbacks;
}

int mavlink_receiver_process(mavlink_receiver_t *receiver, const uint8_t *data, size_t length,
                             uint32_t now_ms)
{
    if (receiver == NULL) {
        return 0;
    }

    receiver->now_ms = now_ms;
    return mavlink_parser_feed(&receiver->parser, data, length, frame_callback, receiver);
}

bool mavlink_receiver_link_update(mavlink_receiver_t *receiver, uint32_t now_ms,
                                  uint32_t timeout_ms)
{
    if (receiver == NULL) {
        return false;
    }

    if (!receiver->heartbeat_received) {
        receiver->link_connected = false;

    } else if ((uint32_t)(now_ms - receiver->last_heartbeat_ms) > timeout_ms) {
        receiver->link_connected = false;
    }

    return receiver->link_connected;
}

bool mavlink_receiver_link_connected(const mavlink_receiver_t *receiver)
{
    return receiver != NULL && receiver->link_connected;
}
