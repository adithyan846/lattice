#include <modules/mavlink/mavlink_mission.h>

#include <stddef.h>
#include <string.h>

#define MAV_FRAME_GLOBAL_RELATIVE_ALT_INT 6

void mavlink_mission_upload_init(mavlink_mission_upload_t *upload)
{
    if (upload == NULL) {
        return;
    }

    memset(upload, 0, sizeof(*upload));
    mission_init(&upload->staging);
}

int mavlink_mission_upload_begin(mavlink_mission_upload_t *upload, uint16_t count)
{
    if (upload == NULL || count == 0 || count > MISSION_MAX_ITEMS) {
        return -1;
    }

    mission_clear(&upload->staging);
    upload->staging.count = count;
    upload->expected_count = count;
    upload->received_count = 0;
    upload->received_mask = 0;
    upload->state = MAVLINK_MISSION_UPLOAD_RECEIVING;
    return 0;
}

int mavlink_mission_upload_item(mavlink_mission_upload_t *upload,
                                const mavlink_mission_item_int_t *item)
{
    if (upload == NULL || item == NULL || upload->state != MAVLINK_MISSION_UPLOAD_RECEIVING ||
        item->seq >= upload->expected_count) {
        return -1;
    }

    if (item->frame != MAV_FRAME_GLOBAL_RELATIVE_ALT_INT) {
        return -1;
    }

    mission_item_t converted;
    memset(&converted, 0, sizeof(converted));
    converted.seq = item->seq;
    converted.command = item->command;
    converted.lat_deg = (double)item->x / 1e7;
    converted.lon_deg = (double)item->y / 1e7;
    converted.alt_m = item->z;
    converted.hold_time_s = item->param1;
    converted.accept_radius_m = item->param2;
    converted.autocontinue = item->autocontinue;

    const uint64_t bit = (uint64_t)1 << item->seq;

    if ((upload->received_mask & bit) == 0) {
        upload->received_mask |= bit;
        upload->received_count++;
    }

    upload->staging.items[item->seq] = converted;

    if (upload->received_count == upload->expected_count) {
        upload->state = MAVLINK_MISSION_UPLOAD_READY;
    }

    return 0;
}

uint8_t mavlink_mission_upload_finish(mavlink_mission_upload_t *upload)
{
    if (upload == NULL || upload->state != MAVLINK_MISSION_UPLOAD_READY) {
        return MAV_MISSION_ERROR;
    }

    if (mission_validate(&upload->staging) != 0) {
        return MAV_MISSION_INVALID;
    }

    return MAV_MISSION_ACCEPTED;
}

void mavlink_mission_upload_clear(mavlink_mission_upload_t *upload)
{
    if (upload == NULL) {
        return;
    }

    mission_clear(&upload->staging);
    upload->received_mask = 0;
    upload->expected_count = 0;
    upload->received_count = 0;
    upload->state = MAVLINK_MISSION_UPLOAD_IDLE;
}

bool mavlink_mission_upload_complete(const mavlink_mission_upload_t *upload)
{
    return upload != NULL && upload->state == MAVLINK_MISSION_UPLOAD_READY;
}

const mission_t *mavlink_mission_upload_mission(const mavlink_mission_upload_t *upload)
{
    return upload == NULL ? NULL : &upload->staging;
}
