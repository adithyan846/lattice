#include <modules/mavlink/mavlink.h>

#include <math.h>
#include <stddef.h>

#include <middleware/log/log.h>
#include <middleware/uorb/uorb.h>
#include <msg/generated/uorb_topics.h>

#define MAVLINK_RX_BUFFER_SIZE 128

static int copy_topic(const char *name, size_t size, void *data)
{
    uorb_topic_t *topic = uorb_topic(name, size);

    if (topic == NULL) {
        return -1;
    }

    return uorb_copy(topic, data);
}

static void publish_telemetry_status(const mavlink_module_t *module, uint32_t now_ms)
{
    uorb_topic_t *topic = uorb_topic("telemetry_status", sizeof(telemetry_status_s));

    if (topic == NULL) {
        return;
    }

    telemetry_status_s status = {0};
    status.timestamp = (uint64_t)now_ms * 1000;
    status.link_type = 1; /* telemetry radio */
    status.connected = mavlink_module_link_connected(module);
    status.link_quality = status.connected ? 1.0f : 0.0f;
    (void)uorb_publish(topic, &status);
}

static void update_heartbeat(mavlink_module_t *module, uint32_t now_ms)
{
    vehicle_status_s status = {0};
    actuator_armed_s armed = {0};
    (void)copy_topic("vehicle_status", sizeof(status), &status);
    (void)copy_topic("actuator_armed", sizeof(armed), &armed);

    mavlink_heartbeat_t heartbeat = {
        .custom_mode = status.nav_state,
        .type = MAV_TYPE_QUADROTOR,
        .autopilot = MAV_AUTOPILOT_GENERIC,
        .base_mode = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED,
        .system_status = armed.armed ? MAV_STATE_ACTIVE : MAV_STATE_STANDBY,
        .mavlink_version = 3,
    };

    if (armed.armed) {
        heartbeat.base_mode |= MAV_MODE_FLAG_SAFETY_ARMED;
    }

    (void)mavlink_sender_heartbeat(&module->sender, &heartbeat);
    (void)now_ms;
}

int mavlink_module_init(mavlink_module_t *module, mavlink_transport_t transport,
                        uint8_t system_id, uint8_t component_id)
{
    if (module == NULL || transport.read == NULL || transport.write == NULL) {
        return -1;
    }

    mavlink_sender_init(&module->sender, transport, system_id, component_id);
    mavlink_receiver_init(&module->receiver, system_id, component_id);

    if (mavlink_streams_default(&module->streams) != 0) {
        return -1;
    }

    module->gcs_timeout_ms = MAVLINK_GCS_TIMEOUT_MS;
    module->initialized = true;
    return 0;
}

void mavlink_module_set_callbacks(mavlink_module_t *module,
                                  const mavlink_receiver_callbacks_t *callbacks)
{
    if (module != NULL) {
        mavlink_receiver_set_callbacks(&module->receiver, callbacks);
    }
}

static void update_streams(mavlink_module_t *module, uint32_t now_ms)
{
    if (mavlink_streams_due(&module->streams, MAVLINK_MSG_ID_HEARTBEAT, now_ms)) {
        update_heartbeat(module, now_ms);
    }

    if (mavlink_streams_due(&module->streams, MAVLINK_MSG_ID_SYS_STATUS, now_ms)) {
        battery_status_s battery = {0};
        telemetry_status_s link = {0};
        (void)copy_topic("battery_status", sizeof(battery), &battery);
        (void)copy_topic("telemetry_status", sizeof(link), &link);

        (void)mavlink_sender_sys_status(
            &module->sender, 0, 0, 0, 0, (uint16_t)(battery.voltage_v * 1000.0f),
            (int16_t)(battery.current_a * 100.0f),
            (int8_t)(battery.remaining * 100.0f));
    }

    if (mavlink_streams_due(&module->streams, MAVLINK_MSG_ID_GPS_RAW_INT, now_ms)) {
        sensor_gps_s gps = {0};

        if (copy_topic("sensor_gps", sizeof(gps), &gps) == 0) {
            const uint16_t speed_cms = (uint16_t)(sqrtf(gps.vel_n * gps.vel_n + gps.vel_e * gps.vel_e) *
                                                  100.0f);
            (void)mavlink_sender_gps_raw_int(
                &module->sender, gps.timestamp, (int32_t)(gps.lat * 1e7), (int32_t)(gps.lon * 1e7),
                (int32_t)(gps.alt * 1000.0f), (uint16_t)(gps.hdop * 100.0f),
                (uint16_t)(gps.vdop * 100.0f), speed_cms,
                (uint16_t)(gps.cog * 100.0f), gps.fix_type, gps.satellites_used);
        }
    }

    if (mavlink_streams_due(&module->streams, MAVLINK_MSG_ID_ATTITUDE, now_ms)) {
        vehicle_attitude_s attitude = {0};

        if (copy_topic("vehicle_attitude", sizeof(attitude), &attitude) == 0) {
            (void)mavlink_sender_attitude(&module->sender, (uint32_t)(attitude.timestamp / 1000),
                                          attitude.roll, attitude.pitch, attitude.yaw, 0.0f, 0.0f,
                                          0.0f);
        }
    }

    if (mavlink_streams_due(&module->streams, MAVLINK_MSG_ID_LOCAL_POSITION_NED, now_ms)) {
        vehicle_local_position_s local = {0};

        if (copy_topic("vehicle_local_position", sizeof(local), &local) == 0) {
            (void)mavlink_sender_local_position_ned(
                &module->sender, (uint32_t)(local.timestamp / 1000), local.x, local.y, local.z,
                local.vx, local.vy, local.vz);
        }
    }

    if (mavlink_streams_due(&module->streams, MAVLINK_MSG_ID_GLOBAL_POSITION_INT, now_ms)) {
        vehicle_global_position_s global = {0};
        home_position_s home = {0};
        (void)copy_topic("vehicle_global_position", sizeof(global), &global);
        (void)copy_topic("home_position", sizeof(home), &home);

        const int32_t relative_alt_mm =
            (int32_t)((global.alt - (home.valid ? home.alt : global.alt)) * 1000.0f);

        if (global.lat != 0.0 || global.lon != 0.0) {
            (void)mavlink_sender_global_position_int(
                &module->sender, (uint32_t)(global.timestamp / 1000), (int32_t)(global.lat * 1e7),
                (int32_t)(global.lon * 1e7), (int32_t)(global.alt * 1000.0f), relative_alt_mm,
                (int16_t)(global.vel_n * 100.0f), (int16_t)(global.vel_e * 100.0f),
                (int16_t)(global.vel_d * 100.0f), 0);
        }
    }

    if (mavlink_streams_due(&module->streams, MAVLINK_MSG_ID_BATTERY_STATUS, now_ms)) {
        battery_status_s battery = {0};

        if (copy_topic("battery_status", sizeof(battery), &battery) == 0) {
            (void)mavlink_sender_battery_status(
                &module->sender, 0, 0, (uint16_t)(battery.voltage_v * 1000.0f),
                (int16_t)(battery.current_a * 100.0f), (int32_t)battery.discharged_mah,
                (int8_t)(battery.remaining * 100.0f));
        }
    }

    if (mavlink_streams_due(&module->streams, MAVLINK_MSG_ID_MISSION_CURRENT, now_ms)) {
        mission_current_s mission = {0};

        if (copy_topic("mission_current", sizeof(mission), &mission) == 0) {
            (void)mavlink_sender_mission_current(&module->sender, mission.seq, mission.total);
        }
    }
}

int mavlink_module_update(mavlink_module_t *module, uint32_t now_ms)
{
    if (module == NULL || !module->initialized) {
        return -1;
    }

    uint8_t buffer[MAVLINK_RX_BUFFER_SIZE];

    for (int iteration = 0; iteration < 8; iteration++) {
        const int received = mavlink_transport_read(&module->sender.transport, buffer, sizeof(buffer));

        if (received <= 0) {
            break;
        }

        (void)mavlink_receiver_process(&module->receiver, buffer, (size_t)received, now_ms);
    }

    (void)mavlink_receiver_link_update(&module->receiver, now_ms, module->gcs_timeout_ms);
    publish_telemetry_status(module, now_ms);
    update_streams(module, now_ms);
    return 0;
}

bool mavlink_module_link_connected(const mavlink_module_t *module)
{
    return module != NULL && mavlink_receiver_link_connected(&module->receiver);
}

int mavlink_start(void)
{
    /* TODO: create the MAVLink task, bind the telemetry UART driver transport,
     * wire receiver callbacks to commander/navigator/params, and call
     * mavlink_module_update() at 50-100 Hz. */
    FRT_LOG_WARN("mavlink: scaffold only, transport task not implemented");
    return -1;
}

int mavlink_stop(void)
{
    return 0;
}
