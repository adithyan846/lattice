#include <modules/sensors/sensors.h>

#include <middleware/log/log.h>
#include <modules/sensors/sensor_aggregator.h>

int sensors_start(void)
{
    if (sensor_aggregator_init() != 0) {
        return -1;
    }

    /* TODO: create the sensors task, initialise the IMU/baro/mag/GNSS drivers,
     * apply calibration, publish sensor_imu/sensor_baro/sensor_mag/sensor_gps
     * at their configured rates, and update sensor_health_t. */
    FRT_LOG_WARN("sensors: scaffold only, driver acquisition not implemented");
    return -1;
}

int sensors_stop(void)
{
    return 0;
}
