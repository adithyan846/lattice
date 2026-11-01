#ifndef SYSTEMCMDS_SHELL_H
#define SYSTEMCMDS_SHELL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHELL_MAX_COMMANDS 24
#define SHELL_MAX_ARGUMENTS 16

typedef int (*shell_command_fn)(int argc, char **argv);

typedef struct {
    const char *name;
    const char *usage;
    const char *help;
    shell_command_fn function;
} shell_command_t;

/** Reset the registry and register the built-in commands. */
int shell_init(void);

int shell_register(const shell_command_t *command);

/** Execute one command line. Returns the command's return value. */
int shell_execute(const char *line);

/** Execute one already-tokenised command. */
int shell_execute_arguments(int argc, char **argv);

size_t shell_command_count(void);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEMCMDS_SHELL_H */
