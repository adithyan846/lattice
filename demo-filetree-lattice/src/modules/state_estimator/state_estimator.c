#include <modules/state_estimator/state_estimator.h>

#include <middleware/log/log.h>

int state_estimator_start(void)
{
    /* TODO: create the 400 Hz estimator task, subscribe to sensor_imu,
     * sensor_baro, sensor_mag and sensor_gps, run the attitude/position
     * estimators, and publish vehicle_attitude, vehicle_local_position,
     * vehicle_global_position and estimator_status. */
    FRT_LOG_WARN("state_estimator: scaffold only, fusion not implemented");
    return -1;
}

int state_estimator_stop(void)
{
    return 0;
}
