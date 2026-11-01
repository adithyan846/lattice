#include <stddef.h>
#include <string.h>

#include <middleware/log/log.h>
#include <middleware/uorb/uorb.h>
#include <modules/commander/commander.h>
#include <msg/generated/uorb_topics.h>
#include <systemcmds/commands.h>

static int mission_command(int argc, char **argv)
{
    if (argc < 2) {
        FRT_LOG_INFO("usage: mission <start|status|pause|resume|abort>");
        return -1;
    }

    if (strcmp(argv[1], "start") == 0) {
        if (commander_set_mode(COMMANDER_NAV_MISSION) != 0) {
            FRT_LOG_WARN("mission: start rejected");
            return -1;
        }

        FRT_LOG_INFO("mission: requested");
        return 0;
    }

    if (strcmp(argv[1], "status") == 0) {
        mission_current_s current = {0};
        uorb_topic_t *topic = uorb_topic("mission_current", sizeof(current));

        if (topic == NULL || uorb_copy(topic, &current) != 0) {
            FRT_LOG_INFO("mission: no mission loaded");
            return -1;
        }

        FRT_LOG_INFO("mission: item %u of %u", current.seq, current.total);
        return 0;
    }

    if (strcmp(argv[1], "pause") == 0 || strcmp(argv[1], "resume") == 0) {
        FRT_LOG_WARN("mission: pause/resume not wired to the navigator yet");
        return -1;
    }

    if (strcmp(argv[1], "abort") == 0) {
        FRT_LOG_WARN("mission: abort not wired to failsafe yet");
        return -1;
    }

    FRT_LOG_WARN("mission: unknown action '%s'", argv[1]);
    return -1;
}

const shell_command_t cmd_mission = {
    .name = "mission",
    .usage = "mission <start|status|pause|resume|abort>",
    .help = "Inspect or control the loaded mission.",
    .function = mission_command,
};
