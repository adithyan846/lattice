#include <modules/commander/commander_state.h>

#include <stddef.h>

void commander_state_init(commander_state_machine_t *machine)
{
    if (machine == NULL) {
        return;
    }

    machine->state = COMMANDER_STATE_BOOT;
    machine->arming = COMMANDER_ARMING_DISARMED;
    machine->nav_state = COMMANDER_NAV_STABILIZED;
    machine->failsafe_active = false;
}

int commander_state_arm(commander_state_machine_t *machine)
{
    if (machine == NULL || machine->failsafe_active ||
        machine->arming == COMMANDER_ARMING_ARMED) {
        return -1;
    }

    machine->arming = COMMANDER_ARMING_ARMED;
    machine->state = COMMANDER_STATE_ARMED;
    return 0;
}

int commander_state_disarm(commander_state_machine_t *machine)
{
    if (machine == NULL) {
        return -1;
    }

    machine->arming = COMMANDER_ARMING_DISARMED;
    machine->state = COMMANDER_STATE_STANDBY;
    machine->failsafe_active = false;
    return 0;
}

int commander_state_set_mode(commander_state_machine_t *machine, commander_nav_state_t mode)
{
    if (machine == NULL || machine->failsafe_active) {
        return -1;
    }

    machine->nav_state = mode;
    return 0;
}

int commander_state_enter_failsafe(commander_state_machine_t *machine)
{
    if (machine == NULL) {
        return -1;
    }

    machine->failsafe_active = true;
    machine->state = COMMANDER_STATE_FAILSAFE;
    return 0;
}

int commander_state_clear_failsafe(commander_state_machine_t *machine, commander_nav_state_t mode)
{
    if (machine == NULL || !machine->failsafe_active) {
        return -1;
    }

    machine->failsafe_active = false;
    machine->state = machine->arming == COMMANDER_ARMING_ARMED ? COMMANDER_STATE_ARMED
                                                              : COMMANDER_STATE_STANDBY;
    machine->nav_state = mode;
    return 0;
}
