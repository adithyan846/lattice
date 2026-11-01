#include <lib/rtl/rtl.h>

#include <math.h>
#include <stddef.h>

void rtl_init(rtl_profile_t *rtl, float return_alt_m, float descend_alt_m)
{
    if (rtl == NULL) {
        return;
    }

    rtl->phase = RTL_PHASE_IDLE;
    rtl->return_alt_m = return_alt_m;
    rtl->descend_alt_m = descend_alt_m;
    rtl->cruise_speed_mps = 5.0f;
    rtl->land_speed_mps = 0.5f;
    rtl->accept_radius_m = 2.0f;
}

rtl_phase_t rtl_update(rtl_profile_t *rtl, const vec3_t *position, const vec3_t *home,
                       bool home_valid)
{
    if (rtl == NULL || position == NULL || home == NULL || !home_valid) {
        return RTL_PHASE_IDLE;
    }

    /* NED: z is down, so altitude is -z. */
    const float altitude = -position->z;
    const float dx = position->x - home->x;
    const float dy = position->y - home->y;
    const float horizontal_distance = sqrtf(dx * dx + dy * dy);

    if (horizontal_distance > rtl->accept_radius_m) {
        /* Away from home: climb to the return altitude, then cruise. */
        rtl->phase = (altitude < rtl->return_alt_m - 1.0f) ? RTL_PHASE_CLIMB : RTL_PHASE_CRUISE;

    } else if (altitude > rtl->descend_alt_m + 0.5f) {
        rtl->phase = RTL_PHASE_DESCEND;

    } else if (altitude > 0.2f) {
        rtl->phase = RTL_PHASE_LAND;

    } else {
        rtl->phase = RTL_PHASE_DONE;
    }

    return rtl->phase;
}

const char *rtl_phase_name(rtl_phase_t phase)
{
    switch (phase) {
    case RTL_PHASE_IDLE:
        return "idle";

    case RTL_PHASE_CLIMB:
        return "climb";

    case RTL_PHASE_CRUISE:
        return "cruise";

    case RTL_PHASE_DESCEND:
        return "descend";

    case RTL_PHASE_LAND:
        return "land";

    case RTL_PHASE_DONE:
        return "done";

    default:
        return "unknown";
    }
}
