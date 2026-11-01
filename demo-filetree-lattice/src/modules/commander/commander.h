#ifndef MODULES_COMMANDER_COMMANDER_H
#define MODULES_COMMANDER_COMMANDER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    COMMANDER_ARMING_DISARMED = 0,
    COMMANDER_ARMING_ARMED,
} commander_arming_state_t;

typedef enum {
    COMMANDER_NAV_MANUAL = 1,
    COMMANDER_NAV_STABILIZED,
    COMMANDER_NAV_ALT_HOLD,
    COMMANDER_NAV_POS_HOLD,
    COMMANDER_NAV_MISSION,
    COMMANDER_NAV_RTL,
    COMMANDER_NAV_LAND,
} commander_nav_state_t;

typedef enum {
    COMMANDER_STATE_BOOT = 0,
    COMMANDER_STATE_STANDBY,
    COMMANDER_STATE_ARMED,
    COMMANDER_STATE_FLIGHT,
    COMMANDER_STATE_FAILSAFE,
    COMMANDER_STATE_LANDING,
} commander_state_t;

int commander_start(void);
int commander_stop(void);

int commander_arm(void);
int commander_disarm(void);
int commander_set_mode(commander_nav_state_t mode);

bool commander_is_armed(void);
const char *commander_mode_name(commander_nav_state_t mode);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_COMMANDER_COMMANDER_H */
