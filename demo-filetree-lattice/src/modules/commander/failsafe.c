#include <modules/commander/failsafe.h>

#include <stddef.h>

failsafe_reason_t failsafe_primary_reason(const failsafe_input_t *input)
{
    if (input == NULL) {
        return FAILSAFE_REASON_NONE;
    }

    if (input->kill_switch) {
        return FAILSAFE_REASON_KILL_SWITCH;
    }

    if (input->battery_critical) {
        return FAILSAFE_REASON_BATTERY_CRITICAL;
    }

    if (input->task_overrun) {
        return FAILSAFE_REASON_WATCHDOG;
    }

    if (input->estimator_invalid) {
        return FAILSAFE_REASON_ESTIMATOR_INVALID;
    }

    if (input->rc_loss) {
        return FAILSAFE_REASON_RC_LOSS;
    }

    if (input->battery_low) {
        return FAILSAFE_REASON_BATTERY_LOW;
    }

    if (input->gnss_loss) {
        return FAILSAFE_REASON_GNSS_LOSS;
    }

    if (input->mission_abort) {
        return FAILSAFE_REASON_MISSION_ABORT;
    }

    if (input->gcs_loss) {
        return FAILSAFE_REASON_GCS_LOSS;
    }

    return FAILSAFE_REASON_NONE;
}

failsafe_action_t failsafe_evaluate(const failsafe_input_t *input)
{
    switch (failsafe_primary_reason(input)) {
    case FAILSAFE_REASON_KILL_SWITCH:
    case FAILSAFE_REASON_BATTERY_CRITICAL:
        return FAILSAFE_ACTION_DISARM;

    case FAILSAFE_REASON_RC_LOSS:
    case FAILSAFE_REASON_BATTERY_LOW:
    case FAILSAFE_REASON_MISSION_ABORT:
        return FAILSAFE_ACTION_RTL;

    case FAILSAFE_REASON_ESTIMATOR_INVALID:
    case FAILSAFE_REASON_WATCHDOG:
        return FAILSAFE_ACTION_LAND;

    case FAILSAFE_REASON_GNSS_LOSS:
        return FAILSAFE_ACTION_HOLD;

    case FAILSAFE_REASON_GCS_LOSS:
        return FAILSAFE_ACTION_NONE;

    case FAILSAFE_REASON_NONE:
    default:
        return FAILSAFE_ACTION_NONE;
    }
}

const char *failsafe_action_name(failsafe_action_t action)
{
    switch (action) {
    case FAILSAFE_ACTION_NONE:
        return "none";

    case FAILSAFE_ACTION_HOLD:
        return "hold";

    case FAILSAFE_ACTION_RTL:
        return "rtl";

    case FAILSAFE_ACTION_LAND:
        return "land";

    case FAILSAFE_ACTION_DISARM:
        return "disarm";

    default:
        return "unknown";
    }
}

const char *failsafe_reason_name(failsafe_reason_t reason)
{
    switch (reason) {
    case FAILSAFE_REASON_NONE:
        return "none";

    case FAILSAFE_REASON_BATTERY_LOW:
        return "battery low";

    case FAILSAFE_REASON_BATTERY_CRITICAL:
        return "battery critical";

    case FAILSAFE_REASON_RC_LOSS:
        return "rc loss";

    case FAILSAFE_REASON_GCS_LOSS:
        return "gcs loss";

    case FAILSAFE_REASON_GNSS_LOSS:
        return "gnss loss";

    case FAILSAFE_REASON_ESTIMATOR_INVALID:
        return "estimator invalid";

    case FAILSAFE_REASON_MISSION_ABORT:
        return "mission abort";

    case FAILSAFE_REASON_WATCHDOG:
        return "watchdog";

    case FAILSAFE_REASON_KILL_SWITCH:
        return "kill switch";

    default:
        return "unknown";
    }
}
