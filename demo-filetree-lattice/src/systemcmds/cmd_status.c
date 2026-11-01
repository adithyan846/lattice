#include <stddef.h>

#include <middleware/log/log.h>
#include <middleware/uorb/uorb.h>
#include <msg/generated/uorb_topics.h>
#include <systemcmds/commands.h>

#include <main/app_config.h>
#include <main/version.h>

static int status_command(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    vehicle_status_s status = {0};
    actuator_armed_s armed = {0};
    battery_status_s battery = {0};
    vehicle_local_position_s local = {0};
    mission_current_s mission = {0};

    uorb_topic_t *status_topic = uorb_topic("vehicle_status", sizeof(status));
    uorb_topic_t *armed_topic = uorb_topic("actuator_armed", sizeof(armed));
    uorb_topic_t *battery_topic = uorb_topic("battery_status", sizeof(battery));
    uorb_topic_t *local_topic = uorb_topic("vehicle_local_position", sizeof(local));
    uorb_topic_t *mission_topic = uorb_topic("mission_current", sizeof(mission));

    if (status_topic != NULL) {
        (void)uorb_copy(status_topic, &status);
    }

    if (armed_topic != NULL) {
        (void)uorb_copy(armed_topic, &armed);
    }

    if (battery_topic != NULL) {
        (void)uorb_copy(battery_topic, &battery);
    }

    if (local_topic != NULL) {
        (void)uorb_copy(local_topic, &local);
    }

    if (mission_topic != NULL) {
        (void)uorb_copy(mission_topic, &mission);
    }

    FRT_LOG_INFO("%s %s", APP_NAME, app_version());
    FRT_LOG_INFO("armed: %s  mode: %u  failsafe: %s", armed.armed ? "yes" : "no", status.nav_state,
                 status.failsafe ? "active" : "clear");
    FRT_LOG_INFO("position: x=%.1f y=%.1f z=%.1f m  heading=%.0f deg", (double)local.x,
                 (double)local.y, (double)local.z, (double)(local.heading * 57.2958f));
    FRT_LOG_INFO("battery: %.2f V  %.1f A  %.0f%%  %s", (double)battery.voltage_v,
                 (double)battery.current_a, (double)(battery.remaining * 100.0f),
                 battery.critical_voltage ? "CRITICAL"
                                          : (battery.low_voltage ? "LOW" : "ok"));
    FRT_LOG_INFO("mission: %u/%u", mission.seq, mission.total);
    return 0;
}

const shell_command_t cmd_status = {
    .name = "status",
    .usage = "status",
    .help = "Print arming, mode, position, battery and mission status.",
    .function = status_command,
};
