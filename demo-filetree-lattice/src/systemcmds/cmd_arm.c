#include <stddef.h>

#include <middleware/log/log.h>
#include <modules/commander/commander.h>
#include <systemcmds/commands.h>

static int arm_command(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (commander_arm() != 0) {
        FRT_LOG_WARN("arm rejected");
        return -1;
    }

    FRT_LOG_INFO("armed");
    return 0;
}

static int disarm_command(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (commander_disarm() != 0) {
        FRT_LOG_WARN("disarm rejected");
        return -1;
    }

    FRT_LOG_INFO("disarmed");
    return 0;
}

const shell_command_t cmd_arm = {
    .name = "arm",
    .usage = "arm",
    .help = "Run arming checks and arm the vehicle.",
    .function = arm_command,
};

const shell_command_t cmd_disarm = {
    .name = "disarm",
    .usage = "disarm",
    .help = "Disarm the vehicle.",
    .function = disarm_command,
};
