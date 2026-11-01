#include <drivers/imu/icm42688p.h>

#include <stddef.h>

#include <drivers/imu/imu_bus.h>

/* ICM-42688-P register map (subset). */
#define ICM42688P_REG_WHO_AM_I 0x75u
#define ICM42688P_WHO_AM_I_VALUE 0x47u
#define ICM42688P_REG_PWR_MGMT0 0x4Eu
#define ICM42688P_REG_GYRO_CONFIG0 0x4Fu
#define ICM42688P_REG_ACCEL_CONFIG0 0x50u
#define ICM42688P_REG_TEMP_DATA1 0x1Du
#define ICM42688P_REG_ACCEL_DATA_X1 0x1Fu
#define ICM42688P_REG_GYRO_DATA_X1 0x25u

int icm42688p_init(void)
{
    /* TODO: reset, verify WHO_AM_I, configure ±16 g / ±2000 dps, set ODR,
     * enable accel+gyro in low-noise mode, configure data-ready interrupt. */
    return -1;
}

int icm42688p_read(imu_sample_t *sample)
{
    if (sample == NULL) {
        return -1;
    }

    /* TODO: burst-read temperature, accel and gyro, convert raw counts to
     * m/s^2 and rad/s, apply calibration, fill timestamp. */
    return -1;
}

static const imu_driver_t icm42688p_driver = {
    .name = "icm42688p",
    .init = icm42688p_init,
    .read = icm42688p_read,
};

const imu_driver_t *imu_icm42688p_get(void)
{
    return &icm42688p_driver;
}
