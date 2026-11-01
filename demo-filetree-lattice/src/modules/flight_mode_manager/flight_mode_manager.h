#ifndef MODULES_FLIGHT_MODE_MANAGER_FLIGHT_MODE_MANAGER_H
#define MODULES_FLIGHT_MODE_MANAGER_FLIGHT_MODE_MANAGER_H

#include <modules/flight_mode_manager/mode.h>

#ifdef __cplusplus
extern "C" {
#endif

int flight_mode_manager_start(void);
int flight_mode_manager_stop(void);

int flight_mode_manager_set_mode(flight_mode_t mode);
flight_mode_t flight_mode_manager_get_mode(void);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_FLIGHT_MODE_MANAGER_FLIGHT_MODE_MANAGER_H */
