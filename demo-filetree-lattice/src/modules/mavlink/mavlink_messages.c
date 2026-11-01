#include <modules/mavlink/mavlink_messages.h>

#include <stddef.h>
#include <string.h>

#include <modules/mavlink/mavlink_encoder.h>

static size_t writer_result(const mavlink_writer_t *writer)
{
    return writer->overflow ? 0 : writer->length;
}

size_t mavlink_pack_heartbeat(uint8_t *payload, size_t capacity, const mavlink_heartbeat_t *message)
{
    if (message == NULL) {
        return 0;
    }

    mavlink_writer_t writer;
    mavlink_writer_init(&writer, payload, capacity);

    mavlink_writer_u32(&writer, message->custom_mode);
    mavlink_writer_u8(&writer, message->type);
    mavlink_writer_u8(&writer, message->autopilot);
    mavlink_writer_u8(&writer, message->base_mode);
    mavlink_writer_u8(&writer, message->system_status);
    mavlink_writer_u8(&writer, message->mavlink_version);
    return writer_result(&writer);
}

size_t mavlink_pack_sys_status(uint8_t *payload, size_t capacity, uint32_t sensors_present,
                               uint32_t sensors_enabled, uint32_t sensors_health,
                               uint16_t load, uint16_t voltage_mv, int16_t current_ca,
                               int8_t battery_remaining)
{
    mavlink_writer_t writer;
    mavlink_writer_init(&writer, payload, capacity);

    mavlink_writer_u32(&writer, sensors_present);
    mavlink_writer_u32(&writer, sensors_enabled);
    mavlink_writer_u32(&writer, sensors_health);
    mavlink_writer_u16(&writer, load);
    mavlink_writer_u16(&writer, voltage_mv);
    mavlink_writer_i16(&writer, current_ca);
    mavlink_writer_u8(&writer, (uint8_t)battery_remaining);
    mavlink_writer_u16(&writer, 0); /* drop_rate_comm */
    mavlink_writer_u16(&writer, 0); /* errors_comm */
    mavlink_writer_u16(&writer, 0); /* errors_count1 */
    mavlink_writer_u16(&writer, 0); /* errors_count2 */
    mavlink_writer_u16(&writer, 0); /* errors_count3 */
    mavlink_writer_u16(&writer, 0); /* errors_count4 */
    return writer_result(&writer);
}

size_t mavlink_pack_gps_raw_int(uint8_t *payload, size_t capacity, uint64_t time_usec,
                                int32_t lat_e7, int32_t lon_e7, int32_t alt_mm, uint16_t eph,
                                uint16_t epv, uint16_t velocity_cms, uint16_t cog_cdeg,
                                uint8_t fix_type, uint8_t satellites)
{
    mavlink_writer_t writer;
    mavlink_writer_init(&writer, payload, capacity);

    mavlink_writer_u64(&writer, time_usec);
    mavlink_writer_i32(&writer, lat_e7);
    mavlink_writer_i32(&writer, lon_e7);
    mavlink_writer_i32(&writer, alt_mm);
    mavlink_writer_u16(&writer, eph);
    mavlink_writer_u16(&writer, epv);
    mavlink_writer_u16(&writer, velocity_cms);
    mavlink_writer_u16(&writer, cog_cdeg);
    mavlink_writer_u8(&writer, fix_type);
    mavlink_writer_u8(&writer, satellites);
    return writer_result(&writer);
}

size_t mavlink_pack_attitude(uint8_t *payload, size_t capacity, uint32_t time_boot_ms, float roll,
                             float pitch, float yaw, float rollspeed, float pitchspeed,
                             float yawspeed)
{
    mavlink_writer_t writer;
    mavlink_writer_init(&writer, payload, capacity);

    mavlink_writer_u32(&writer, time_boot_ms);
    mavlink_writer_f32(&writer, roll);
    mavlink_writer_f32(&writer, pitch);
    mavlink_writer_f32(&writer, yaw);
    mavlink_writer_f32(&writer, rollspeed);
    mavlink_writer_f32(&writer, pitchspeed);
    mavlink_writer_f32(&writer, yawspeed);
    return writer_result(&writer);
}

size_t mavlink_pack_local_position_ned(uint8_t *payload, size_t capacity, uint32_t time_boot_ms,
                                       float x, float y, float z, float vx, float vy, float vz)
{
    mavlink_writer_t writer;
    mavlink_writer_init(&writer, payload, capacity);

    mavlink_writer_u32(&writer, time_boot_ms);
    mavlink_writer_f32(&writer, x);
    mavlink_writer_f32(&writer, y);
    mavlink_writer_f32(&writer, z);
    mavlink_writer_f32(&writer, vx);
    mavlink_writer_f32(&writer, vy);
    mavlink_writer_f32(&writer, vz);
    return writer_result(&writer);
}

size_t mavlink_pack_global_position_int(uint8_t *payload, size_t capacity, uint32_t time_boot_ms,
                                        int32_t lat_e7, int32_t lon_e7, int32_t alt_mm,
                                        int32_t relative_alt_mm, int16_t vx_cms, int16_t vy_cms,
                                        int16_t vz_cms, uint16_t heading_cdeg)
{
    mavlink_writer_t writer;
    mavlink_writer_init(&writer, payload, capacity);

    mavlink_writer_u32(&writer, time_boot_ms);
    mavlink_writer_i32(&writer, lat_e7);
    mavlink_writer_i32(&writer, lon_e7);
    mavlink_writer_i32(&writer, alt_mm);
    mavlink_writer_i32(&writer, relative_alt_mm);
    mavlink_writer_i16(&writer, vx_cms);
    mavlink_writer_i16(&writer, vy_cms);
    mavlink_writer_i16(&writer, vz_cms);
    mavlink_writer_u16(&writer, heading_cdeg);
    return writer_result(&writer);
}

size_t mavlink_pack_battery_status(uint8_t *payload, size_t capacity, uint8_t id,
                                   int16_t temperature_cdeg, uint16_t voltage_mv,
                                   int16_t current_ca, int32_t consumed_mah,
                                   int8_t battery_remaining)
{
    mavlink_writer_t writer;
    mavlink_writer_init(&writer, payload, capacity);

    mavlink_writer_u8(&writer, id);
    mavlink_writer_u8(&writer, 0); /* battery_function: unknown */
    mavlink_writer_u8(&writer, 0); /* type: unknown */
    mavlink_writer_i16(&writer, temperature_cdeg);

    for (int cell = 0; cell < 10; cell++) {
        mavlink_writer_u16(&writer, cell == 0 ? voltage_mv : 0);
    }

    mavlink_writer_i16(&writer, current_ca);
    mavlink_writer_i32(&writer, consumed_mah);
    mavlink_writer_i32(&writer, 0); /* energy_consumed */
    mavlink_writer_u8(&writer, (uint8_t)battery_remaining);
    return writer_result(&writer);
}

size_t mavlink_pack_mission_current(uint8_t *payload, size_t capacity, uint16_t seq,
                                    uint16_t total)
{
    mavlink_writer_t writer;
    mavlink_writer_init(&writer, payload, capacity);

    mavlink_writer_u16(&writer, seq);
    mavlink_writer_u16(&writer, total);
    return writer_result(&writer);
}

size_t mavlink_pack_mission_count(uint8_t *payload, size_t capacity, const mavlink_mission_count_t *message)
{
    if (message == NULL) {
        return 0;
    }

    mavlink_writer_t writer;
    mavlink_writer_init(&writer, payload, capacity);

    mavlink_writer_u16(&writer, message->count);
    mavlink_writer_u8(&writer, message->target_system);
    mavlink_writer_u8(&writer, message->target_component);
    mavlink_writer_u8(&writer, message->mission_type);
    return writer_result(&writer);
}

size_t mavlink_pack_mission_request_int(uint8_t *payload, size_t capacity,
                                        const mavlink_mission_request_t *message)
{
    if (message == NULL) {
        return 0;
    }

    mavlink_writer_t writer;
    mavlink_writer_init(&writer, payload, capacity);

    mavlink_writer_u8(&writer, message->target_system);
    mavlink_writer_u8(&writer, message->target_component);
    mavlink_writer_u16(&writer, message->seq);
    mavlink_writer_u8(&writer, message->mission_type);
    return writer_result(&writer);
}

size_t mavlink_pack_mission_ack(uint8_t *payload, size_t capacity, const mavlink_mission_ack_t *message)
{
    if (message == NULL) {
        return 0;
    }

    mavlink_writer_t writer;
    mavlink_writer_init(&writer, payload, capacity);

    mavlink_writer_u8(&writer, message->target_system);
    mavlink_writer_u8(&writer, message->target_component);
    mavlink_writer_u8(&writer, message->type);
    mavlink_writer_u8(&writer, message->mission_type);
    return writer_result(&writer);
}

size_t mavlink_pack_command_ack(uint8_t *payload, size_t capacity, uint16_t command, uint8_t result)
{
    mavlink_writer_t writer;
    mavlink_writer_init(&writer, payload, capacity);

    mavlink_writer_u16(&writer, command);
    mavlink_writer_u8(&writer, result);
    return writer_result(&writer);
}

size_t mavlink_pack_statustext(uint8_t *payload, size_t capacity, uint8_t severity, const char *text)
{
    mavlink_writer_t writer;
    mavlink_writer_init(&writer, payload, capacity);

    mavlink_writer_u8(&writer, severity);

    for (int index = 0; index < 50; index++) {
        const char character = (text != NULL && text[index] != '\0') ? text[index] : '\0';
        mavlink_writer_u8(&writer, (uint8_t)character);
    }

    return writer_result(&writer);
}

bool mavlink_unpack_heartbeat(const uint8_t *payload, size_t length, mavlink_heartbeat_t *message)
{
    if (payload == NULL || message == NULL || length < 9) {
        return false;
    }

    mavlink_reader_t reader;
    mavlink_reader_init(&reader, payload, length);

    message->custom_mode = mavlink_reader_u32(&reader);
    message->type = mavlink_reader_u8(&reader);
    message->autopilot = mavlink_reader_u8(&reader);
    message->base_mode = mavlink_reader_u8(&reader);
    message->system_status = mavlink_reader_u8(&reader);
    message->mavlink_version = mavlink_reader_u8(&reader);
    return mavlink_reader_ok(&reader);
}

bool mavlink_unpack_command_long(const uint8_t *payload, size_t length,
                                 mavlink_command_long_t *message)
{
    if (payload == NULL || message == NULL || length < 33) {
        return false;
    }

    mavlink_reader_t reader;
    mavlink_reader_init(&reader, payload, length);

    message->param1 = mavlink_reader_f32(&reader);
    message->param2 = mavlink_reader_f32(&reader);
    message->param3 = mavlink_reader_f32(&reader);
    message->param4 = mavlink_reader_f32(&reader);
    message->param5 = mavlink_reader_f32(&reader);
    message->param6 = mavlink_reader_f32(&reader);
    message->param7 = mavlink_reader_f32(&reader);
    message->command = mavlink_reader_u16(&reader);
    message->target_system = mavlink_reader_u8(&reader);
    message->target_component = mavlink_reader_u8(&reader);
    message->confirmation = mavlink_reader_u8(&reader);
    return mavlink_reader_ok(&reader);
}

bool mavlink_unpack_mission_count(const uint8_t *payload, size_t length,
                                  mavlink_mission_count_t *message)
{
    if (payload == NULL || message == NULL || length < 5) {
        return false;
    }

    mavlink_reader_t reader;
    mavlink_reader_init(&reader, payload, length);

    message->count = mavlink_reader_u16(&reader);
    message->target_system = mavlink_reader_u8(&reader);
    message->target_component = mavlink_reader_u8(&reader);
    message->mission_type = mavlink_reader_u8(&reader);
    return mavlink_reader_ok(&reader);
}

bool mavlink_unpack_mission_request(const uint8_t *payload, size_t length,
                                    mavlink_mission_request_t *message)
{
    if (payload == NULL || message == NULL || length < 5) {
        return false;
    }

    mavlink_reader_t reader;
    mavlink_reader_init(&reader, payload, length);

    message->target_system = mavlink_reader_u8(&reader);
    message->target_component = mavlink_reader_u8(&reader);
    message->seq = mavlink_reader_u16(&reader);
    message->mission_type = mavlink_reader_u8(&reader);
    return mavlink_reader_ok(&reader);
}

bool mavlink_unpack_mission_item_int(const uint8_t *payload, size_t length,
                                     mavlink_mission_item_int_t *message)
{
    if (payload == NULL || message == NULL || length < 38) {
        return false;
    }

    mavlink_reader_t reader;
    mavlink_reader_init(&reader, payload, length);

    message->target_system = mavlink_reader_u8(&reader);
    message->target_component = mavlink_reader_u8(&reader);
    message->seq = mavlink_reader_u16(&reader);
    message->frame = mavlink_reader_u8(&reader);
    message->command = mavlink_reader_u16(&reader);
    message->current = mavlink_reader_u8(&reader);
    message->autocontinue = mavlink_reader_u8(&reader);
    message->param1 = mavlink_reader_f32(&reader);
    message->param2 = mavlink_reader_f32(&reader);
    message->param3 = mavlink_reader_f32(&reader);
    message->param4 = mavlink_reader_f32(&reader);
    message->x = mavlink_reader_i32(&reader);
    message->y = mavlink_reader_i32(&reader);
    message->z = mavlink_reader_f32(&reader);
    message->mission_type = mavlink_reader_u8(&reader);
    return mavlink_reader_ok(&reader);
}

bool mavlink_unpack_param_set(const uint8_t *payload, size_t length, mavlink_param_set_t *message)
{
    if (payload == NULL || message == NULL || length < 23) {
        return false;
    }

    mavlink_reader_t reader;
    mavlink_reader_init(&reader, payload, length);

    for (int index = 0; index < 16; index++) {
        message->param_id[index] = (char)mavlink_reader_u8(&reader);
    }

    message->param_id[16] = '\0';
    message->param_value = mavlink_reader_f32(&reader);
    message->param_type = mavlink_reader_u8(&reader);
    message->target_system = mavlink_reader_u8(&reader);
    message->target_component = mavlink_reader_u8(&reader);
    return mavlink_reader_ok(&reader);
}
