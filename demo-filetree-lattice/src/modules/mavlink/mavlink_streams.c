#include <modules/mavlink/mavlink_streams.h>

#include <stddef.h>
#include <string.h>

#include <modules/mavlink/mavlink_messages.h>

void mavlink_streams_init(mavlink_streams_t *streams)
{
    if (streams == NULL) {
        return;
    }

    memset(streams, 0, sizeof(*streams));
}

int mavlink_streams_set(mavlink_streams_t *streams, uint32_t message_id, uint32_t interval_ms)
{
    if (streams == NULL || interval_ms == 0) {
        return -1;
    }

    for (uint8_t index = 0; index < streams->count; index++) {
        if (streams->streams[index].message_id == message_id) {
            streams->streams[index].interval_ms = interval_ms;
            return 0;
        }
    }

    if (streams->count >= MAVLINK_STREAM_MAX) {
        return -1;
    }

    streams->streams[streams->count].message_id = message_id;
    streams->streams[streams->count].interval_ms = interval_ms;
    streams->streams[streams->count].last_sent_ms = 0;
    streams->count++;
    return 0;
}

int mavlink_streams_default(mavlink_streams_t *streams)
{
    if (streams == NULL) {
        return -1;
    }

    mavlink_streams_init(streams);

    const struct {
        uint32_t message_id;
        uint32_t interval_ms;
    } defaults[] = {
        {MAVLINK_MSG_ID_HEARTBEAT, 1000},
        {MAVLINK_MSG_ID_SYS_STATUS, 500},
        {MAVLINK_MSG_ID_GPS_RAW_INT, 200},
        {MAVLINK_MSG_ID_ATTITUDE, 100},
        {MAVLINK_MSG_ID_LOCAL_POSITION_NED, 100},
        {MAVLINK_MSG_ID_GLOBAL_POSITION_INT, 200},
        {MAVLINK_MSG_ID_BATTERY_STATUS, 1000},
        {MAVLINK_MSG_ID_MISSION_CURRENT, 500},
    };

    for (size_t index = 0; index < sizeof(defaults) / sizeof(defaults[0]); index++) {
        if (mavlink_streams_set(streams, defaults[index].message_id, defaults[index].interval_ms) !=
            0) {
            return -1;
        }
    }

    return 0;
}

bool mavlink_streams_due(mavlink_streams_t *streams, uint32_t message_id, uint32_t now_ms)
{
    if (streams == NULL) {
        return false;
    }

    for (uint8_t index = 0; index < streams->count; index++) {
        mavlink_stream_t *stream = &streams->streams[index];

        if (stream->message_id != message_id) {
            continue;
        }

        if ((uint32_t)(now_ms - stream->last_sent_ms) >= stream->interval_ms) {
            stream->last_sent_ms = now_ms;
            return true;
        }

        return false;
    }

    return false;
}
