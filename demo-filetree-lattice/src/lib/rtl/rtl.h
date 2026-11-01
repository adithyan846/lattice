#ifndef LIB_RTL_RTL_H
#define LIB_RTL_RTL_H

#include <stdbool.h>

#include <lib/math/vec3.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RTL_PHASE_IDLE = 0,
    RTL_PHASE_CLIMB,
    RTL_PHASE_CRUISE,
    RTL_PHASE_DESCEND,
    RTL_PHASE_LAND,
    RTL_PHASE_DONE,
} rtl_phase_t;

typedef struct {
    rtl_phase_t phase;
    float return_alt_m;
    float descend_alt_m;
    float cruise_speed_mps;
    float land_speed_mps;
    float accept_radius_m;
} rtl_profile_t;

void rtl_init(rtl_profile_t *rtl, float return_alt_m, float descend_alt_m);

/**
 * Advance the RTL phase from the current local position and home position.
 * This function only chooses a phase; the navigator turns it into setpoints.
 */
rtl_phase_t rtl_update(rtl_profile_t *rtl, const vec3_t *position, const vec3_t *home,
                       bool home_valid);

const char *rtl_phase_name(rtl_phase_t phase);

#ifdef __cplusplus
}
#endif

#endif /* LIB_RTL_RTL_H */
