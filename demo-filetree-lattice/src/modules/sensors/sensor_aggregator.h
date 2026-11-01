#ifndef MODULES_SENSORS_SENSOR_AGGREGATOR_H
#define MODULES_SENSORS_SENSOR_AGGREGATOR_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool imu_valid;
    bool baro_valid;
    bool mag_valid;
    bool gps_valid;
} sensor_health_t;

int sensor_aggregator_init(void);
sensor_health_t sensor_aggregator_health(void);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_SENSORS_SENSOR_AGGREGATOR_H */
