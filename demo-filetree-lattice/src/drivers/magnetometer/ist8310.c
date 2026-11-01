#include <drivers/magnetometer/ist8310.h>

#include <stddef.h>

#include <hal/i2c.h>

#define IST8310_I2C_ADDRESS 0x0Eu
#define IST8310_REG_WHO_AM_I 0x00u
#define IST8310_WHO_AM_I_VALUE 0x10u
#define IST8310_REG_DATA_X_L 0x03u

int ist8310_init(void)
{
    /* TODO: verify WHO_AM_I, configure continuous measurement mode and range. */
    return -1;
}

int ist8310_read(mag_sample_t *sample)
{
    if (sample == NULL) {
        return -1;
    }

    /* TODO: burst-read six data bytes, convert to gauss, apply calibration. */
    return -1;
}

static const mag_driver_t ist8310_driver = {
    .name = "ist8310",
    .init = ist8310_init,
    .read = ist8310_read,
};

const mag_driver_t *mag_ist8310_get(void)
{
    return &ist8310_driver;
}
