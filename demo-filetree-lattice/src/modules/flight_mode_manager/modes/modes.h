#ifndef MODULES_FLIGHT_MODE_MANAGER_MODES_MODES_H
#define MODULES_FLIGHT_MODE_MANAGER_MODES_MODES_H

#include <modules/flight_mode_manager/mode.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const flight_mode_ops_t mode_manual_ops;
extern const flight_mode_ops_t mode_stabilized_ops;
extern const flight_mode_ops_t mode_alt_hold_ops;
extern const flight_mode_ops_t mode_pos_hold_ops;
extern const flight_mode_ops_t mode_mission_ops;
extern const flight_mode_ops_t mode_rtl_ops;
extern const flight_mode_ops_t mode_land_ops;

/**
 * Override the RTL profile used by mode_rtl_ops. Must be called before the mode
 * is entered. Defaults are 30 m return altitude and 5 m descend altitude.
 */
int mode_rtl_configure(float return_alt_m, float descend_alt_m);

/** Override the land descent speed in m/s. Default is 0.5 m/s. */
int mode_land_configure(float descent_speed_mps);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_FLIGHT_MODE_MANAGER_MODES_MODES_H */
