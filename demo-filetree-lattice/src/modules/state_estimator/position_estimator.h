#ifndef MODULES_STATE_ESTIMATOR_POSITION_ESTIMATOR_H
#define MODULES_STATE_ESTIMATOR_POSITION_ESTIMATOR_H

#include <stdbool.h>

#include <lib/math/vec3.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    vec3_t position_ned_m;
    vec3_t velocity_ned_mps;
    bool local_valid;
    bool global_valid;
    double lat_deg;
    double lon_deg;
    float alt_m;
} position_estimator_t;

void position_estimator_init(position_estimator_t *estimator);

int position_estimator_update(position_estimator_t *estimator, const vec3_t *accel_ned,
                              float baro_alt_m, bool baro_valid, const void *gnss_sample,
                              float dt_s);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_STATE_ESTIMATOR_POSITION_ESTIMATOR_H */
