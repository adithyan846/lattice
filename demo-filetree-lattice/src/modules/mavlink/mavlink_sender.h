#ifndef MODULES_MAVLINK_MAVLINK_SENDER_H
#define MODULES_MAVLINK_MAVLINK_SENDER_H

#include <stddef.h>
#include <stdint.h>

#include <modules/mavlink/mavlink_messages.h>
#include <modules/mavlink/mavlink_transport.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t sequence;
    uint8_t system_id;
    uint8_t component_id;
    mavlink_transport_t transport;
} mavlink_sender_t;

void mavlink_sender_init(mavlink_sender_t *sender, mavlink_transport_t transport, uint8_t system_id,
                         uint8_t component_id);

/** Encode and transmit one message. Returns 0 on success. */
int mavlink_sender_send(mavlink_sender_t *sender, uint32_t message_id, const uint8_t *payload,
                        uint8_t payload_length);

int mavlink_sender_heartbeat(mavlink_sender_t *sender, const mavlink_heartbeat_t *message);
int mavlink_sender_sys_status(mavlink_sender_t *sender, uint32_t sensors_present,
                              uint32_t sensors_enabled, uint32_t sensors_health, uint16_t load,
                              uint16_t voltage_mv, int16_t current_ca, int8_t battery_remaining);
int mavlink_sender_gps_raw_int(mavlink_sender_t *sender, uint64_t time_usec, int32_t lat_e7,
                               int32_t lon_e7, int32_t alt_mm, uint16_t eph, uint16_t epv,
                               uint16_t velocity_cms, uint16_t cog_cdeg, uint8_t fix_type,
                               uint8_t satellites);
int mavlink_sender_attitude(mavlink_sender_t *sender, uint32_t time_boot_ms, float roll, float pitch,
                            float yaw, float rollspeed, float pitchspeed, float yawspeed);
int mavlink_sender_local_position_ned(mavlink_sender_t *sender, uint32_t time_boot_ms, float x,
                                      float y, float z, float vx, float vy, float vz);
int mavlink_sender_global_position_int(mavlink_sender_t *sender, uint32_t time_boot_ms, int32_t lat_e7,
                                       int32_t lon_e7, int32_t alt_mm, int32_t relative_alt_mm,
                                       int16_t vx_cms, int16_t vy_cms, int16_t vz_cms,
                                       uint16_t heading_cdeg);
int mavlink_sender_battery_status(mavlink_sender_t *sender, uint8_t id, int16_t temperature_cdeg,
                                  uint16_t voltage_mv, int16_t current_ca, int32_t consumed_mah,
                                  int8_t battery_remaining);
int mavlink_sender_mission_current(mavlink_sender_t *sender, uint16_t seq, uint16_t total);
int mavlink_sender_command_ack(mavlink_sender_t *sender, uint16_t command, uint8_t result);
int mavlink_sender_statustext(mavlink_sender_t *sender, uint8_t severity, const char *text);
int mavlink_sender_mission_count(mavlink_sender_t *sender,
                                 const mavlink_mission_count_t *message);
int mavlink_sender_mission_request_int(mavlink_sender_t *sender,
                                       const mavlink_mission_request_t *message);
int mavlink_sender_mission_ack(mavlink_sender_t *sender, const mavlink_mission_ack_t *message);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_MAVLINK_MAVLINK_SENDER_H */
