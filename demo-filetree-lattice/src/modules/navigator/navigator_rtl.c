#include <modules/navigator/navigator_rtl.h>

#include <stddef.h>
#include <string.h>

void navigator_rtl_init(navigator_rtl_t *navigator, float return_alt_m, float descend_alt_m)
{
    if (navigator == NULL) {
        return;
    }

    memset(navigator, 0, sizeof(*navigator));
    rtl_init(&navigator->profile, return_alt_m, descend_alt_m);
}

int navigator_rtl_start(navigator_rtl_t *navigator, const vec3_t *home_ned)
{
    if (navigator == NULL || home_ned == NULL) {
        return -1;
    }

    navigator->home_ned = *home_ned;
    navigator->home_valid = true;
    navigator->active = true;
    navigator->target_valid = false;
    navigator->profile.phase = RTL_PHASE_IDLE;
    return 0;
}

void navigator_rtl_stop(navigator_rtl_t *navigator)
{
    if (navigator != NULL) {
        navigator->active = false;
        navigator->target_valid = false;
        navigator->profile.phase = RTL_PHASE_IDLE;
    }
}

rtl_phase_t navigator_rtl_update(navigator_rtl_t *navigator, const vec3_t *position_ned)
{
    if (navigator == NULL || position_ned == NULL || !navigator->active ||
        !navigator->home_valid) {
        return RTL_PHASE_IDLE;
    }

    const rtl_phase_t phase =
        rtl_update(&navigator->profile, position_ned, &navigator->home_ned, true);

    vec3_t target = *position_ned;

    switch (phase) {
    case RTL_PHASE_CLIMB:
        target.z = navigator->home_ned.z - navigator->profile.return_alt_m;
        break;

    case RTL_PHASE_CRUISE:
        target.x = navigator->home_ned.x;
        target.y = navigator->home_ned.y;
        target.z = navigator->home_ned.z - navigator->profile.return_alt_m;
        break;

    case RTL_PHASE_DESCEND:
        target.x = navigator->home_ned.x;
        target.y = navigator->home_ned.y;
        target.z = navigator->home_ned.z - navigator->profile.descend_alt_m;
        break;

    case RTL_PHASE_LAND:
    case RTL_PHASE_DONE:
        target = navigator->home_ned;
        break;

    case RTL_PHASE_IDLE:
    default:
        navigator->target_valid = false;
        return RTL_PHASE_IDLE;
    }

    navigator->target_ned = target;
    navigator->target_valid = true;
    return phase;
}

const vec3_t *navigator_rtl_target(const navigator_rtl_t *navigator)
{
    if (navigator == NULL || !navigator->target_valid) {
        return NULL;
    }

    return &navigator->target_ned;
}
