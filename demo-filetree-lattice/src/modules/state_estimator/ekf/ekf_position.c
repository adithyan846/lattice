#include <modules/state_estimator/ekf/ekf_position.h>

#include <stddef.h>

void ekf_position_init(ekf_position_t *ekf)
{
    (void)ekf;
}

int ekf_position_predict(ekf_position_t *ekf, const float accel_ned[3], float dt_s)
{
    (void)ekf;
    (void)accel_ned;
    (void)dt_s;
    return -1;
}

int ekf_position_update_baro(ekf_position_t *ekf, float altitude_m)
{
    (void)ekf;
    (void)altitude_m;
    return -1;
}

int ekf_position_update_gnss(ekf_position_t *ekf, double lat, double lon, float alt,
                             const float velocity_ned[3])
{
    (void)ekf;
    (void)lat;
    (void)lon;
    (void)alt;
    (void)velocity_ned;
    return -1;
}
