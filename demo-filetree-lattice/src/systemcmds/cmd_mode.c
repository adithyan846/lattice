#include <stddef.h>
#include <string.h>

#include <middleware/log/log.h>
#include <modules/commander/commander.h>
#include <systemcmds/commands.h>

static int mode_command(int argc, char **argv)
{
    if (argc < 2) {
        FRT_LOG_INFO("usage: mode <manual|stabilized|alt|pos|mission|rtl|land>");
        return -1;
    }

    commander_nav_state_t mode;

    if (strcmp(argv[1], "manual") == 0) {
        mode = COMMANDER_NAV_MANUAL;

    } else if (strcmp(argv[1], "stabilized") == 0) {
        mode = COMMANDER_NAV_STABILIZED;

    } else if (strcmp(argv[1], "alt") == 0) {
        mode = COMMANDER_NAV_ALT_HOLD;

    } else if (strcmp(argv[1], "pos") == 0) {
        mode = COMMANDER_NAV_POS_HOLD;

    } else if (strcmp(argv[1], "mission") == 0) {
        mode = COMMANDER_NAV_MISSION;

    } else if (strcmp(argv[1], "rtl") == 0) {
        mode = COMMANDER_NAV_RTL;

    } else if (strcmp(argv[1], "land") == 0) {
        mode = COMMANDER_NAV_LAND;

    } else {
        FRT_LOG_WARN("mode: unknown mode '%s'", argv[1]);
        return -1;
    }

    if (commander_set_mode(mode) != 0) {
        FRT_LOG_WARN("mode: change rejected");
        return -1;
    }

    FRT_LOG_INFO("mode: %s", commander_mode_name(mode));
    return 0;
}

const shell_command_t cmd_mode = {
    .name = "mode",
    .usage = "mode <manual|stabilized|alt|pos|mission|rtl|land>",
    .help = "Change flight mode through the commander.",
    .function = mode_command,
};
