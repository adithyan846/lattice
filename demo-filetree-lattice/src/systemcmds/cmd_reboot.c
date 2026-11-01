#include <stddef.h>

#include <board.h>
#include <middleware/log/log.h>
#include <systemcmds/commands.h>

static int reboot_command(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    FRT_LOG_WARN("rebooting");
    board_reset();
    return 0;
}

const shell_command_t cmd_reboot = {
    .name = "reboot",
    .usage = "reboot",
    .help = "Reset the autopilot.",
    .function = reboot_command,
};
