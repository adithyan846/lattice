#include <stddef.h>
#include <string.h>

#include <middleware/log/log.h>
#include <modules/logger/logger.h>
#include <systemcmds/commands.h>

static int logger_command(int argc, char **argv)
{
    if (argc < 2) {
        FRT_LOG_INFO("usage: logger <start|stop|erase|status>");
        return -1;
    }

    if (strcmp(argv[1], "status") == 0) {
        FRT_LOG_INFO("logger: scaffold only, no active log");
        return 0;
    }

    if (strcmp(argv[1], "start") == 0) {
        FRT_LOG_WARN("logger: start requires the storage backend");
        return logger_start();
    }

    if (strcmp(argv[1], "stop") == 0) {
        return logger_stop();
    }

    if (strcmp(argv[1], "erase") == 0) {
        FRT_LOG_WARN("logger: erase is not implemented");
        return -1;
    }

    FRT_LOG_WARN("logger: unknown action '%s'", argv[1]);
    return -1;
}

const shell_command_t cmd_logger = {
    .name = "logger",
    .usage = "logger <start|stop|erase|status>",
    .help = "Control flight logging.",
    .function = logger_command,
};
