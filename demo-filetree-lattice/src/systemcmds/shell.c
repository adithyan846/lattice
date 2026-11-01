#include <systemcmds/shell.h>

#include <stddef.h>
#include <string.h>

#include <middleware/log/log.h>

#include <systemcmds/commands.h>
#include <systemcmds/shell.h>

static const shell_command_t *commands[SHELL_MAX_COMMANDS];
static size_t command_count;

int shell_register(const shell_command_t *command)
{
    if (command == NULL || command->name == NULL || command->function == NULL ||
        command_count >= SHELL_MAX_COMMANDS) {
        return -1;
    }

    commands[command_count++] = command;
    return 0;
}

int shell_init(void)
{
    command_count = 0;

    if (shell_register(&cmd_status) != 0 || shell_register(&cmd_arm) != 0 ||
        shell_register(&cmd_disarm) != 0 || shell_register(&cmd_mode) != 0 ||
        shell_register(&cmd_mission) != 0 || shell_register(&cmd_params) != 0 ||
        shell_register(&cmd_logger) != 0 || shell_register(&cmd_reboot) != 0) {
        return -1;
    }

    return 0;
}

int shell_execute_arguments(int argc, char **argv)
{
    if (argc <= 0 || argv == NULL || argv[0] == NULL) {
        return -1;
    }

    for (size_t index = 0; index < command_count; index++) {
        if (strcmp(commands[index]->name, argv[0]) == 0) {
            return commands[index]->function(argc, argv);
        }
    }

    FRT_LOG_WARN("shell: unknown command '%s'", argv[0]);
    return -1;
}

int shell_execute(const char *line)
{
    if (line == NULL) {
        return -1;
    }

    char buffer[128];
    const size_t length = strlen(line);

    if (length >= sizeof(buffer)) {
        FRT_LOG_WARN("shell: command line too long");
        return -1;
    }

    memcpy(buffer, line, length + 1);

    char *argv[SHELL_MAX_ARGUMENTS];
    int argc = 0;
    char *cursor = buffer;

    while (*cursor != '\0' && argc < SHELL_MAX_ARGUMENTS) {
        while (*cursor == ' ' || *cursor == '\t') {
            cursor++;
        }

        if (*cursor == '\0') {
            break;
        }

        argv[argc++] = cursor;

        while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t') {
            cursor++;
        }

        if (*cursor != '\0') {
            *cursor++ = '\0';
        }
    }

    return shell_execute_arguments(argc, argv);
}

size_t shell_command_count(void)
{
    return command_count;
}
