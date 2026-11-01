#ifndef MODULES_COMMANDER_COMMANDER_STATE_H
#define MODULES_COMMANDER_COMMANDER_STATE_H

#include <stdbool.h>

#include <modules/commander/commander.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    commander_state_t state;
    commander_arming_state_t arming;
    commander_nav_state_t nav_state;
    bool failsafe_active;
} commander_state_machine_t;

void commander_state_init(commander_state_machine_t *machine);
int commander_state_arm(commander_state_machine_t *machine);
int commander_state_disarm(commander_state_machine_t *machine);
int commander_state_set_mode(commander_state_machine_t *machine, commander_nav_state_t mode);
int commander_state_enter_failsafe(commander_state_machine_t *machine);
int commander_state_clear_failsafe(commander_state_machine_t *machine, commander_nav_state_t mode);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_COMMANDER_COMMANDER_STATE_H */
