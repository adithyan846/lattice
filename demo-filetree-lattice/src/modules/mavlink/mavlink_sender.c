#include <modules/mavlink/mavlink_sender.h>

#include <stddef.h>

#include <modules/mavlink/mavlink_encoder.h>

#define MAVLINK_TX_BUFFER_SIZE 300

void mavlink_sender_init(mavlink_sender_t *sender, mavlink_transport_t transport, uint8_t system_id,
                         uint8_t component_id)
{
    if (sender == NULL) {
        return;
    }

    sender->sequence = 0;
    sender->system_id = system_id;
    sender->component_id = component_id;
    sender->transport = transport;
}

int mavlink_sender_send(mavlink_sender_t *sender, uint32_t message_id, const uint8_t *payload,
                        uint8_t payload_length)
{
    if (sender == NULL) {
        return -1;
    }

    uint8_t frame[MAVLINK_TX_BUFFER_SIZE];
    size_t frame_length;

    if (mavlink_encode_v2(sender->sequence, sender->system_id, sender->component_id, message_id,
                          payload, payload_length, frame, sizeof(frame), &frame_length) != 0) {
        return -1;
    }

    if (mavlink_transport_write_all(&sender->transport, frame, frame_length) != 0) {
        return -1;
    }

    sender->sequence++;
    return 0;
}

int mavlink_sender_heartbeat(mavlink_sender_t *sender, const mavlink_heartbeat_t *message)
{
    uint8_t payload[16];
    const size_t length = mavlink_pack_heartbeat(payload, sizeof(payload), message);
    return length == 0 ? -1
                       : mavlink_sender_send(sender, MAVLINK_MSG_ID_HEARTBEAT, payload,
                                             (uint8_t)length);
}

int mavlink_sender_sys_status(mavlink_sender_t *sender, uint32_t sensors_present,
                              uint32_t sensors_enabled, uint32_t sensors_health, uint16_t load,
                              uint16_t voltage_mv, int16_t current_ca, int8_t battery_remaining)
{
    uint8_t payload[40];
    const size_t length =
        mavlink_pack_sys_status(payload, sizeof(payload), sensors_present, sensors_enabled,
                                sensors_health, load, voltage_mv, current_ca, battery_remaining);
    return length == 0 ? -1
                       : mavlink_sender_send(sender, MAVLINK_MSG_ID_SYS_STATUS, payload,
                                             (uint8_t)length);
}

int mavlink_sender_gps_raw_int(mavlink_sender_t *sender, uint64_t time_usec, int32_t lat_e7,
                               int32_t lon_e7, int32_t alt_mm, uint16_t eph, uint16_t epv,
                               uint16_t velocity_cms, uint16_t cog_cdeg, uint8_t fix_type,
                               uint8_t satellites)
{
    uint8_t payload[40];
    const size_t length =
        mavlink_pack_gps_raw_int(payload, sizeof(payload), time_usec, lat_e7, lon_e7, alt_mm, eph,
                                 epv, velocity_cms, cog_cdeg, fix_type, satellites);
    return length == 0 ? -1
                       : mavlink_sender_send(sender, MAVLINK_MSG_ID_GPS_RAW_INT, payload,
                                             (uint8_t)length);
}

int mavlink_sender_attitude(mavlink_sender_t *sender, uint32_t time_boot_ms, float roll, float pitch,
                            float yaw, float rollspeed, float pitchspeed, float yawspeed)
{
    uint8_t payload[32];
    const size_t length = mavlink_pack_attitude(payload, sizeof(payload), time_boot_ms, roll, pitch,
                                                yaw, rollspeed, pitchspeed, yawspeed);
    return length == 0 ? -1
                       : mavlink_sender_send(sender, MAVLINK_MSG_ID_ATTITUDE, payload,
                                             (uint8_t)length);
}

int mavlink_sender_local_position_ned(mavlink_sender_t *sender, uint32_t time_boot_ms, float x,
                                      float y, float z, float vx, float vy, float vz)
{
    uint8_t payload[32];
    const size_t length =
        mavlink_pack_local_position_ned(payload, sizeof(payload), time_boot_ms, x, y, z, vx, vy, vz);
    return length == 0 ? -1
                       : mavlink_sender_send(sender, MAVLINK_MSG_ID_LOCAL_POSITION_NED, payload,
                                             (uint8_t)length);
}

int mavlink_sender_global_position_int(mavlink_sender_t *sender, uint32_t time_boot_ms, int32_t lat_e7,
                                       int32_t lon_e7, int32_t alt_mm, int32_t relative_alt_mm,
                                       int16_t vx_cms, int16_t vy_cms, int16_t vz_cms,
                                       uint16_t heading_cdeg)
{
    uint8_t payload[40];
    const size_t length = mavlink_pack_global_position_int(
        payload, sizeof(payload), time_boot_ms, lat_e7, lon_e7, alt_mm, relative_alt_mm, vx_cms,
        vy_cms, vz_cms, heading_cdeg);
    return length == 0 ? -1
                       : mavlink_sender_send(sender, MAVLINK_MSG_ID_GLOBAL_POSITION_INT, payload,
                                             (uint8_t)length);
}

int mavlink_sender_battery_status(mavlink_sender_t *sender, uint8_t id, int16_t temperature_cdeg,
                                  uint16_t voltage_mv, int16_t current_ca, int32_t consumed_mah,
                                  int8_t battery_remaining)
{
    uint8_t payload[48];
    const size_t length =
        mavlink_pack_battery_status(payload, sizeof(payload), id, temperature_cdeg, voltage_mv,
                                    current_ca, consumed_mah, battery_remaining);
    return length == 0 ? -1
                       : mavlink_sender_send(sender, MAVLINK_MSG_ID_BATTERY_STATUS, payload,
                                             (uint8_t)length);
}

int mavlink_sender_mission_current(mavlink_sender_t *sender, uint16_t seq, uint16_t total)
{
    uint8_t payload[8];
    const size_t length = mavlink_pack_mission_current(payload, sizeof(payload), seq, total);
    return length == 0 ? -1
                       : mavlink_sender_send(sender, MAVLINK_MSG_ID_MISSION_CURRENT, payload,
                                             (uint8_t)length);
}

int mavlink_sender_command_ack(mavlink_sender_t *sender, uint16_t command, uint8_t result)
{
    uint8_t payload[8];
    const size_t length = mavlink_pack_command_ack(payload, sizeof(payload), command, result);
    return length == 0 ? -1
                       : mavlink_sender_send(sender, MAVLINK_MSG_ID_COMMAND_ACK, payload,
                                             (uint8_t)length);
}

int mavlink_sender_statustext(mavlink_sender_t *sender, uint8_t severity, const char *text)
{
    uint8_t payload[64];
    const size_t length = mavlink_pack_statustext(payload, sizeof(payload), severity, text);
    return length == 0 ? -1
                       : mavlink_sender_send(sender, MAVLINK_MSG_ID_STATUSTEXT, payload,
                                             (uint8_t)length);
}

int mavlink_sender_mission_count(mavlink_sender_t *sender,
                                 const mavlink_mission_count_t *message)
{
    uint8_t payload[8];
    const size_t length = mavlink_pack_mission_count(payload, sizeof(payload), message);
    return length == 0 ? -1
                       : mavlink_sender_send(sender, MAVLINK_MSG_ID_MISSION_COUNT, payload,
                                             (uint8_t)length);
}

int mavlink_sender_mission_request_int(mavlink_sender_t *sender,
                                       const mavlink_mission_request_t *message)
{
    uint8_t payload[8];
    const size_t length = mavlink_pack_mission_request_int(payload, sizeof(payload), message);
    return length == 0 ? -1
                       : mavlink_sender_send(sender, MAVLINK_MSG_ID_MISSION_REQUEST_INT, payload,
                                             (uint8_t)length);
}

int mavlink_sender_mission_ack(mavlink_sender_t *sender, const mavlink_mission_ack_t *message)
{
    uint8_t payload[8];
    const size_t length = mavlink_pack_mission_ack(payload, sizeof(payload), message);
    return length == 0 ? -1
                       : mavlink_sender_send(sender, MAVLINK_MSG_ID_MISSION_ACK, payload,
                                             (uint8_t)length);
}
