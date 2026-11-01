#ifndef MODULES_MAVLINK_MAVLINK_MESSAGES_H
#define MODULES_MAVLINK_MAVLINK_MESSAGES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Message IDs used by the scaffold. */
#define MAVLINK_MSG_ID_HEARTBEAT 0
#define MAVLINK_MSG_ID_SYS_STATUS 1
#define MAVLINK_MSG_ID_SYSTEM_TIME 2
#define MAVLINK_MSG_ID_PARAM_SET 23
#define MAVLINK_MSG_ID_GPS_RAW_INT 24
#define MAVLINK_MSG_ID_ATTITUDE 30
#define MAVLINK_MSG_ID_LOCAL_POSITION_NED 32
#define MAVLINK_MSG_ID_GLOBAL_POSITION_INT 33
#define MAVLINK_MSG_ID_MISSION_SET_CURRENT 41
#define MAVLINK_MSG_ID_MISSION_CURRENT 42
#define MAVLINK_MSG_ID_MISSION_REQUEST_LIST 43
#define MAVLINK_MSG_ID_MISSION_COUNT 44
#define MAVLINK_MSG_ID_MISSION_CLEAR_ALL 45
#define MAVLINK_MSG_ID_MISSION_ACK 47
#define MAVLINK_MSG_ID_MISSION_REQUEST_INT 51
#define MAVLINK_MSG_ID_MISSION_ITEM_INT 73
#define MAVLINK_MSG_ID_COMMAND_LONG 76
#define MAVLINK_MSG_ID_COMMAND_ACK 77
#define MAVLINK_MSG_ID_BATTERY_STATUS 147
#define MAVLINK_MSG_ID_STATUSTEXT 253

/* Command IDs. */
#define MAV_CMD_NAV_WAYPOINT 16
#define MAV_CMD_NAV_RETURN_TO_LAUNCH 20
#define MAV_CMD_NAV_LAND 21
#define MAV_CMD_NAV_TAKEOFF 22
#define MAV_CMD_DO_SET_MODE 176
#define MAV_CMD_DO_SET_HOME 179
#define MAV_CMD_DO_PAUSE_CONTINUE 193
#define MAV_CMD_COMPONENT_ARM_DISARM 400

/* MAV_RESULT values used in COMMAND_ACK. */
#define MAV_RESULT_ACCEPTED 0
#define MAV_RESULT_TEMPORARILY_REJECTED 1
#define MAV_RESULT_DENIED 2
#define MAV_RESULT_UNSUPPORTED 3
#define MAV_RESULT_FAILED 4

/* MAV_MISSION_RESULT values used in MISSION_ACK. */
#define MAV_MISSION_ACCEPTED 0
#define MAV_MISSION_ERROR 1
#define MAV_MISSION_UNSUPPORTED 2
#define MAV_MISSION_NO_SPACE 3
#define MAV_MISSION_INVALID 4

/* MAV_MODE_FLAG bits. */
#define MAV_MODE_FLAG_SAFETY_ARMED 0x80
#define MAV_MODE_FLAG_CUSTOM_MODE_ENABLED 0x01

/* MAV_AUTOPILOT / MAV_TYPE placeholders for this project. */
#define MAV_AUTOPILOT_GENERIC 0
#define MAV_TYPE_QUADROTOR 2
#define MAV_STATE_STANDBY 3
#define MAV_STATE_ACTIVE 4

typedef struct {
    uint32_t custom_mode;
    uint8_t type;
    uint8_t autopilot;
    uint8_t base_mode;
    uint8_t system_status;
    uint8_t mavlink_version;
} mavlink_heartbeat_t;

typedef struct {
    uint16_t command;
    float param1;
    float param2;
    float param3;
    float param4;
    float param5;
    float param6;
    float param7;
    uint8_t target_system;
    uint8_t target_component;
    uint8_t confirmation;
} mavlink_command_long_t;

typedef struct {
    uint8_t target_system;
    uint8_t target_component;
    uint16_t seq;
    uint8_t frame;
    uint16_t command;
    uint8_t current;
    uint8_t autocontinue;
    float param1;
    float param2;
    float param3;
    float param4;
    int32_t x;
    int32_t y;
    float z;
    uint8_t mission_type;
} mavlink_mission_item_int_t;

typedef struct {
    uint16_t count;
    uint8_t target_system;
    uint8_t target_component;
    uint8_t mission_type;
} mavlink_mission_count_t;

typedef struct {
    uint8_t target_system;
    uint8_t target_component;
    uint16_t seq;
    uint8_t mission_type;
} mavlink_mission_request_t;

typedef struct {
    uint8_t target_system;
    uint8_t target_component;
    uint8_t type;
    uint8_t mission_type;
} mavlink_mission_ack_t;

typedef struct {
    char param_id[17];
    float param_value;
    uint8_t param_type;
    uint8_t target_system;
    uint8_t target_component;
} mavlink_param_set_t;

/* Payload encoders. Return the payload length, or 0 on error. */
size_t mavlink_pack_heartbeat(uint8_t *payload, size_t capacity, const mavlink_heartbeat_t *message);
size_t mavlink_pack_sys_status(uint8_t *payload, size_t capacity, uint32_t sensors_present,
                               uint32_t sensors_enabled, uint32_t sensors_health,
                               uint16_t load, uint16_t voltage_mv, int16_t current_ca,
                               int8_t battery_remaining);
size_t mavlink_pack_gps_raw_int(uint8_t *payload, size_t capacity, uint64_t time_usec,
                                int32_t lat_e7, int32_t lon_e7, int32_t alt_mm, uint16_t eph,
                                uint16_t epv, uint16_t velocity_cms, uint16_t cog_cdeg,
                                uint8_t fix_type, uint8_t satellites);
size_t mavlink_pack_attitude(uint8_t *payload, size_t capacity, uint32_t time_boot_ms, float roll,
                             float pitch, float yaw, float rollspeed, float pitchspeed,
                             float yawspeed);
size_t mavlink_pack_local_position_ned(uint8_t *payload, size_t capacity, uint32_t time_boot_ms,
                                       float x, float y, float z, float vx, float vy, float vz);
size_t mavlink_pack_global_position_int(uint8_t *payload, size_t capacity, uint32_t time_boot_ms,
                                        int32_t lat_e7, int32_t lon_e7, int32_t alt_mm,
                                        int32_t relative_alt_mm, int16_t vx_cms, int16_t vy_cms,
                                        int16_t vz_cms, uint16_t heading_cdeg);
size_t mavlink_pack_battery_status(uint8_t *payload, size_t capacity, uint8_t id,
                                   int16_t temperature_cdeg, uint16_t voltage_mv,
                                   int16_t current_ca, int32_t consumed_mah,
                                   int8_t battery_remaining);
size_t mavlink_pack_mission_current(uint8_t *payload, size_t capacity, uint16_t seq,
                                    uint16_t total);
size_t mavlink_pack_mission_count(uint8_t *payload, size_t capacity, const mavlink_mission_count_t *message);
size_t mavlink_pack_mission_request_int(uint8_t *payload, size_t capacity,
                                        const mavlink_mission_request_t *message);
size_t mavlink_pack_mission_ack(uint8_t *payload, size_t capacity, const mavlink_mission_ack_t *message);
size_t mavlink_pack_command_ack(uint8_t *payload, size_t capacity, uint16_t command, uint8_t result);
size_t mavlink_pack_statustext(uint8_t *payload, size_t capacity, uint8_t severity, const char *text);

/* Payload decoders. Return true when the payload is long enough. */
bool mavlink_unpack_heartbeat(const uint8_t *payload, size_t length, mavlink_heartbeat_t *message);
bool mavlink_unpack_command_long(const uint8_t *payload, size_t length,
                                 mavlink_command_long_t *message);
bool mavlink_unpack_mission_count(const uint8_t *payload, size_t length,
                                  mavlink_mission_count_t *message);
bool mavlink_unpack_mission_request(const uint8_t *payload, size_t length,
                                    mavlink_mission_request_t *message);
bool mavlink_unpack_mission_item_int(const uint8_t *payload, size_t length,
                                     mavlink_mission_item_int_t *message);
bool mavlink_unpack_param_set(const uint8_t *payload, size_t length, mavlink_param_set_t *message);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_MAVLINK_MAVLINK_MESSAGES_H */
