#include <modules/state_estimator/position_estimator.h>

#include <stddef.h>

void position_estimator_init(position_estimator_t *estimator)
{
    if (estimator == NULL) {
        return;
    }

    estimator->position_ned_m = vec3_zero();
    estimator->velocity_ned_mps = vec3_zero();
    estimator->local_valid = false;
    estimator->global_valid = false;
    estimator->lat_deg = 0.0;
    estimator->lon_deg = 0.0;
    estimator->alt_m = 0.0f;
}

int position_estimator_update(position_estimator_t *estimator, const vec3_t *accel_ned,
                              float baro_alt_m, bool baro_valid, const void *gnss_sample,
                              float dt_s)
{
    (void)estimator;
    (void)accel_ned;
    (void)baro_alt_m;
    (void)baro_valid;
    (void)gnss_sample;
    (void)dt_s;

    /* TODO: dead-reckon accel, fuse baro for altitude and GNSS for
     * horizontal position/velocity with innovation gating. */
    return -1;
}
