#ifndef MODULES_MAVLINK_MAVLINK_RECEIVER_H
#define MODULES_MAVLINK_MAVLINK_RECEIVER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <lib/mission/mission.h>
#include <modules/mavlink/mavlink_commands.h>
#include <modules/mavlink/mavlink_mission.h>
#include <modules/mavlink/mavlink_parser.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void *context;

    void (*command)(const mavlink_command_t *command, void *context);
    void (*mission_request)(uint16_t seq, void *context);
    void (*mission_item_requested)(uint16_t seq, void *context);
    void (*mission_download_requested)(void *context);
    void (*mission_upload_finished)(uint8_t result, const mission_t *mission, void *context);
    void (*mission_cleared)(void *context);
    void (*mission_set_current)(uint16_t seq, void *context);
    void (*param_set)(const char *name, float value, uint8_t type, void *context);
} mavlink_receiver_callbacks_t;

typedef struct {
    mavlink_parser_t parser;
    mavlink_mission_upload_t upload;
    mavlink_receiver_callbacks_t callbacks;
    uint32_t last_heartbeat_ms;
    uint32_t now_ms;
    uint8_t system_id;
    uint8_t component_id;
    bool heartbeat_received;
    bool link_connected;
} mavlink_receiver_t;

void mavlink_receiver_init(mavlink_receiver_t *receiver, uint8_t system_id, uint8_t component_id);
void mavlink_receiver_set_callbacks(mavlink_receiver_t *receiver,
                                    const mavlink_receiver_callbacks_t *callbacks);

/**
 * Feed bytes from the GCS. Returns the number of valid frames processed.
 * `now_ms` is used for heartbeat/link-loss tracking.
 */
int mavlink_receiver_process(mavlink_receiver_t *receiver, const uint8_t *data, size_t length,
                             uint32_t now_ms);

/**
 * Update link state from heartbeat age. Returns true when the GCS link is up.
 * `timeout_ms` defaults to 5000 in the caller configuration.
 */
bool mavlink_receiver_link_update(mavlink_receiver_t *receiver, uint32_t now_ms,
                                  uint32_t timeout_ms);

bool mavlink_receiver_link_connected(const mavlink_receiver_t *receiver);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_MAVLINK_MAVLINK_RECEIVER_H */
