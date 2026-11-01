#ifndef MODULES_MAVLINK_MAVLINK_MISSION_H
#define MODULES_MAVLINK_MAVLINK_MISSION_H

#include <stdbool.h>
#include <stdint.h>

#include <lib/mission/mission.h>
#include <modules/mavlink/mavlink_messages.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MAVLINK_MISSION_UPLOAD_IDLE = 0,
    MAVLINK_MISSION_UPLOAD_RECEIVING,
    MAVLINK_MISSION_UPLOAD_READY,
} mavlink_mission_upload_state_t;

typedef struct {
    mission_t staging;
    uint64_t received_mask;
    uint16_t expected_count;
    uint16_t received_count;
    mavlink_mission_upload_state_t state;
} mavlink_mission_upload_t;

void mavlink_mission_upload_init(mavlink_mission_upload_t *upload);

/** Start an upload. Returns 0 on success, -1 on invalid count. */
int mavlink_mission_upload_begin(mavlink_mission_upload_t *upload, uint16_t count);

/**
 * Store one MISSION_ITEM_INT. Returns 0 on success, -1 on invalid item.
 * The upload moves to READY once every sequence has been received.
 */
int mavlink_mission_upload_item(mavlink_mission_upload_t *upload,
                                const mavlink_mission_item_int_t *item);

/** Validate the completed upload and return a MAV_MISSION_* result code. */
uint8_t mavlink_mission_upload_finish(mavlink_mission_upload_t *upload);

void mavlink_mission_upload_clear(mavlink_mission_upload_t *upload);

bool mavlink_mission_upload_complete(const mavlink_mission_upload_t *upload);
const mission_t *mavlink_mission_upload_mission(const mavlink_mission_upload_t *upload);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_MAVLINK_MAVLINK_MISSION_H */
