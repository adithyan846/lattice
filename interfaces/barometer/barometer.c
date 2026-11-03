#include "barometer.h"
#include "bmp280.h"
#include <stddef.h>

/*
 * BAROMETER INTERFACE
 *
 * Device selection and update dispatch:
 *     Currently BMP280
 *
 * Data handling:
 *     Receives measurements from the device driver
 *     Stores the latest measurement
 *     Provides data to application/core consumers
 */

static barometer_data_t latest_data;
static bool data_available = false;

int barometer_init(void)
{
    return bmp280_init();
}

int barometer_update(void)
{
    return bmp280_update();
}

void barometer_publish(const barometer_data_t *data)
{
    if (data == NULL)
        return;

    latest_data = *data;
    latest_data.valid = true;
    data_available = true;
}

bool barometer_get_latest(barometer_data_t *data)
{
    if (data == NULL || !data_available)
        return false;

    *data = latest_data;
    return true;
}
