#ifndef MODULES_COMMANDER_FAILSAFE_H
#define MODULES_COMMANDER_FAILSAFE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool battery_low;
    bool battery_critical;
    bool rc_loss;
    bool gcs_loss;
    bool gnss_loss;
    bool estimator_invalid;
    bool mission_abort;
    bool task_overrun;
    bool kill_switch;
} failsafe_input_t;

typedef enum {
    FAILSAFE_REASON_NONE = 0,
    FAILSAFE_REASON_BATTERY_LOW,
    FAILSAFE_REASON_BATTERY_CRITICAL,
    FAILSAFE_REASON_RC_LOSS,
    FAILSAFE_REASON_GCS_LOSS,
    FAILSAFE_REASON_GNSS_LOSS,
    FAILSAFE_REASON_ESTIMATOR_INVALID,
    FAILSAFE_REASON_MISSION_ABORT,
    FAILSAFE_REASON_WATCHDOG,
    FAILSAFE_REASON_KILL_SWITCH,
} failsafe_reason_t;

typedef enum {
    FAILSAFE_ACTION_NONE = 0,
    FAILSAFE_ACTION_HOLD,
    FAILSAFE_ACTION_RTL,
    FAILSAFE_ACTION_LAND,
    FAILSAFE_ACTION_DISARM,
} failsafe_action_t;

failsafe_action_t failsafe_evaluate(const failsafe_input_t *input);
failsafe_reason_t failsafe_primary_reason(const failsafe_input_t *input);
const char *failsafe_action_name(failsafe_action_t action);
const char *failsafe_reason_name(failsafe_reason_t reason);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_COMMANDER_FAILSAFE_H */
