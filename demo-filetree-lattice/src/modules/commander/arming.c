#include <modules/commander/arming.h>

#include <stddef.h>

arming_result_t arming_check(const arming_checks_t *checks)
{
    if (checks == NULL) {
        return ARMING_ATTITUDE_INVALID;
    }

    if (!checks->no_failsafe) {
        return ARMING_FAILSAFE_ACTIVE;
    }

    if (!checks->attitude_valid) {
        return ARMING_ATTITUDE_INVALID;
    }

    if (!checks->gyro_calibrated) {
        return ARMING_GYRO_NOT_CALIBRATED;
    }

    if (!checks->accel_calibrated) {
        return ARMING_ACCEL_NOT_CALIBRATED;
    }

    if (!checks->mag_calibrated) {
        return ARMING_MAG_NOT_CALIBRATED;
    }

    if (!checks->gnss_ok) {
        return ARMING_GNSS_NOT_READY;
    }

    if (!checks->battery_ok) {
        return ARMING_BATTERY_LOW;
    }

    if (!checks->rc_ok) {
        return ARMING_RC_NOT_READY;
    }

    if (!checks->mission_loaded && checks->mission_required) {
        return ARMING_MISSION_NOT_LOADED;
    }

    return ARMING_OK;
}

const char *arming_result_name(arming_result_t result)
{
    switch (result) {
    case ARMING_OK:
        return "ok";

    case ARMING_ATTITUDE_INVALID:
        return "attitude invalid";

    case ARMING_GYRO_NOT_CALIBRATED:
        return "gyro not calibrated";

    case ARMING_ACCEL_NOT_CALIBRATED:
        return "accel not calibrated";

    case ARMING_MAG_NOT_CALIBRATED:
        return "mag not calibrated";

    case ARMING_GNSS_NOT_READY:
        return "gnss not ready";

    case ARMING_BATTERY_LOW:
        return "battery low";

    case ARMING_RC_NOT_READY:
        return "rc not ready";

    case ARMING_FAILSAFE_ACTIVE:
        return "failsafe active";

    case ARMING_MISSION_NOT_LOADED:
        return "mission not loaded";

    default:
        return "unknown";
    }
}
