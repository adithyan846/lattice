#include <drivers/barometer/ms5611.h>

#include <stddef.h>

#include <hal/i2c.h>

#define MS5611_I2C_ADDRESS 0x77u
#define MS5611_CMD_RESET 0x1Eu
#define MS5611_CMD_PROM_READ 0xA0u
#define MS5611_CMD_CONVERT_D1 0x48u
#define MS5611_CMD_CONVERT_D2 0x58u
#define MS5611_CMD_ADC_READ 0x00u

int ms5611_init(void)
{
    /* TODO: reset, wait for PROM, read calibration coefficients, verify CRC. */
    return -1;
}

int ms5611_read(baro_sample_t *sample)
{
    if (sample == NULL) {
        return -1;
    }

    /* TODO: trigger D1/D2 conversions, read ADC, apply second-order
     * temperature compensation, fill pressure/temperature. */
    return -1;
}

static const baro_driver_t ms5611_driver = {
    .name = "ms5611",
    .init = ms5611_init,
    .read = ms5611_read,
};

const baro_driver_t *baro_ms5611_get(void)
{
    return &ms5611_driver;
}
