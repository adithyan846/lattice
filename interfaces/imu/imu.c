#include "imu.h"
#include "lsm6dso.h"
#include "board.h"
#include <stddef.h>

/*
 * IMU INTERFACE
 *
 * Device selection and update dispatch:
 *     Currently LSM6DSO on BOARD_SPI_IMU_BUS
 *
 * Data handling:
 *     Receives measurements from the device driver
 *     Stores the latest measurement
 *     Provides data to application/core consumers
 */

static imu_data_t latest_data;
static bool data_available = false;

int imu_init(void)
{
    return lsm6dso_init(BOARD_SPI_IMU_BUS);
}

/* 0 = new sample published, LSM6DSO_NO_NEW_DATA = nothing new, <0 = error */
int imu_update(void)
{
    return lsm6dso_update();
}

void imu_publish(const imu_data_t *data)
{
    if (data == NULL)
        return;

    latest_data = *data;
    latest_data.valid = true;
    data_available = true;
}

bool imu_get_latest(imu_data_t *data)
{
    if (data == NULL || !data_available)
        return false;

    *data = latest_data;
    return true;
}
