#ifndef MODULES_NAVIGATOR_NAVIGATOR_RTL_H
#define MODULES_NAVIGATOR_NAVIGATOR_RTL_H

#include <stdbool.h>

#include <lib/math/vec3.h>
#include <lib/rtl/rtl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    rtl_profile_t profile;
    vec3_t home_ned;
    vec3_t target_ned;
    bool home_valid;
    bool active;
    bool target_valid;
} navigator_rtl_t;

void navigator_rtl_init(navigator_rtl_t *navigator, float return_alt_m, float descend_alt_m);

int navigator_rtl_start(navigator_rtl_t *navigator, const vec3_t *home_ned);

void navigator_rtl_stop(navigator_rtl_t *navigator);

/**
 * Advance the RTL phase and compute the next position target. Returns the
 * current phase; RTL_PHASE_DONE means the vehicle is home and on the ground.
 */
rtl_phase_t navigator_rtl_update(navigator_rtl_t *navigator, const vec3_t *position_ned);

const vec3_t *navigator_rtl_target(const navigator_rtl_t *navigator);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_NAVIGATOR_NAVIGATOR_RTL_H */
