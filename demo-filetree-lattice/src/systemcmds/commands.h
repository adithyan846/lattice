#ifndef SYSTEMCMDS_COMMANDS_H
#define SYSTEMCMDS_COMMANDS_H

#include <systemcmds/shell.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const shell_command_t cmd_status;
extern const shell_command_t cmd_arm;
extern const shell_command_t cmd_disarm;
extern const shell_command_t cmd_mode;
extern const shell_command_t cmd_mission;
extern const shell_command_t cmd_params;
extern const shell_command_t cmd_logger;
extern const shell_command_t cmd_reboot;

#ifdef __cplusplus
}
#endif

#endif /* SYSTEMCMDS_COMMANDS_H */
