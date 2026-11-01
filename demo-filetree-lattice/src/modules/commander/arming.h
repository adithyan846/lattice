#ifndef MODULES_COMMANDER_ARMING_H
#define MODULES_COMMANDER_ARMING_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool attitude_valid;
    bool gyro_calibrated;
    bool accel_calibrated;
    bool mag_calibrated;
    bool gnss_ok;
    bool battery_ok;
    bool rc_ok;
    bool no_failsafe;
    bool mission_required;
    bool mission_loaded;
} arming_checks_t;

typedef enum {
    ARMING_OK = 0,
    ARMING_ATTITUDE_INVALID,
    ARMING_GYRO_NOT_CALIBRATED,
    ARMING_ACCEL_NOT_CALIBRATED,
    ARMING_MAG_NOT_CALIBRATED,
    ARMING_GNSS_NOT_READY,
    ARMING_BATTERY_LOW,
    ARMING_RC_NOT_READY,
    ARMING_FAILSAFE_ACTIVE,
    ARMING_MISSION_NOT_LOADED,
} arming_result_t;

arming_result_t arming_check(const arming_checks_t *checks);
const char *arming_result_name(arming_result_t result);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_COMMANDER_ARMING_H */
