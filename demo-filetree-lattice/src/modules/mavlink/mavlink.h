#ifndef MODULES_MAVLINK_MAVLINK_H
#define MODULES_MAVLINK_MAVLINK_H

#include <stdbool.h>
#include <stdint.h>

#include <modules/mavlink/mavlink_receiver.h>
#include <modules/mavlink/mavlink_sender.h>
#include <modules/mavlink/mavlink_streams.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAVLINK_SYSTEM_ID 1
#define MAVLINK_COMPONENT_ID 1
#define MAVLINK_GCS_TIMEOUT_MS 5000

typedef struct {
    mavlink_sender_t sender;
    mavlink_receiver_t receiver;
    mavlink_streams_t streams;
    uint32_t gcs_timeout_ms;
    bool initialized;
} mavlink_module_t;

/**
 * Wire the MAVLink module to a transport. The transport must stay alive for the
 * lifetime of the module. Returns 0 on success.
 */
int mavlink_module_init(mavlink_module_t *module, mavlink_transport_t transport,
                        uint8_t system_id, uint8_t component_id);

void mavlink_module_set_callbacks(mavlink_module_t *module,
                                  const mavlink_receiver_callbacks_t *callbacks);

/**
 * Service the link: read available bytes, dispatch commands/mission items, and
 * transmit any streams that are due. Call from the MAVLink task (typically at
 * 50-100 Hz); this function never blocks.
 */
int mavlink_module_update(mavlink_module_t *module, uint32_t now_ms);

bool mavlink_module_link_connected(const mavlink_module_t *module);

int mavlink_start(void);
int mavlink_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_MAVLINK_MAVLINK_H */
