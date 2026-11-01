#include <modules/sensors/sensor_aggregator.h>

#include <string.h>

static sensor_health_t health;

int sensor_aggregator_init(void)
{
    memset(&health, 0, sizeof(health));
    return 0;
}

sensor_health_t sensor_aggregator_health(void)
{
    return health;
}
