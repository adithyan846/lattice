#include "bmp280.h"
#include "barometer.h"

#include "hal_timer.h"
#include "hal_spi.h"

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

/* Static device instance */
static bmp280_dev_t bmp280_device;

/* ============================================================================
 * BASIC HELPERS
 * ========================================================================== */

static void delay_ms(uint32_t ms)
{
    hal_timer_delay_ms(ms);
}

/* ============================================================================
 * SPI REGISTER OPERATIONS (VEGA SDK Pattern)
 * ========================================================================== */

static int bmp280_read_regs(uint8_t reg, uint8_t *buf, uint8_t len)
{
    if (buf == NULL || len == 0)
        return -1;

    uint8_t cmd = reg | BMP280_SPI_READ;

    return hal_spi_write_read(
        BMP280_SPI_BUS,
        &cmd,
        1,
        buf,
        len,
        BMP280_TIMEOUT_MS
    );
}

static int bmp280_write_reg(uint8_t reg, uint8_t value)
{
    uint8_t tx[2] = { reg & BMP280_SPI_WRITE_MASK, value };

    return hal_spi_write(
        BMP280_SPI_BUS,
        tx,
        2,
        BMP280_TIMEOUT_MS
    );
}

/* ============================================================================
 * CALIBRATION DATA DECODING
 * ========================================================================== */

static uint16_t le16u(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static int16_t le16s(const uint8_t *p)
{
    return (int16_t)le16u(p);
}

static void bmp280_decode_calibration(bmp280_calib_t *calib, const uint8_t *data)
{
    calib->dig_t1 = le16u(&data[0]);
    calib->dig_t2 = le16s(&data[2]);
    calib->dig_t3 = le16s(&data[4]);

    calib->dig_p1 = le16u(&data[6]);
    calib->dig_p2 = le16s(&data[8]);
    calib->dig_p3 = le16s(&data[10]);
    calib->dig_p4 = le16s(&data[12]);
    calib->dig_p5 = le16s(&data[14]);
    calib->dig_p6 = le16s(&data[16]);
    calib->dig_p7 = le16s(&data[18]);
    calib->dig_p8 = le16s(&data[20]);
    calib->dig_p9 = le16s(&data[22]);
}

static int bmp280_calib_valid(const bmp280_calib_t *c)
{
    if (c->dig_t1 == 0 || c->dig_p1 == 0)
        return 0;
    if (c->dig_t1 == 0xFFFF || c->dig_p1 == 0xFFFF)
        return 0;
    if (c->dig_t2 == 0 && c->dig_t3 == 0)
        return 0;
    return 1;
}

static int bmp280_read_calibration(bmp280_dev_t *dev)
{
    uint8_t calib_data[24];

    if (bmp280_read_regs(BMP280_REG_CALIB, calib_data, sizeof(calib_data)) != 0)
        return -1;

    bmp280_decode_calibration(&dev->calib, calib_data);

    if (!bmp280_calib_valid(&dev->calib))
        return -2;

    return 0;
}

/* ============================================================================
 * DEVICE INITIALISATION (VEGA SDK Pattern: begin)
 * ========================================================================== */

static int bmp280_init_device(bmp280_dev_t *dev)
{
    uint8_t id;

    if (dev == NULL)
        return -1;

    /* Reset state */
    dev->t_fine = 0;
    dev->last_temp_centi = 0;
    dev->last_pressure_pa = 0;
    dev->ready = false;

    delay_ms(BMP280_RESET_DELAY_MS);

    /* Verify chip ID */
    if (bmp280_read_regs(BMP280_REG_ID, &id, 1) != 0)
        return -2;

    if (id != BMP280_CHIP_ID)
        return -3;

    /* Soft reset */
    if (bmp280_write_reg(BMP280_REG_RESET, BMP280_RESET_VALUE) != 0)
        return -4;

    delay_ms(BMP280_RESET_DELAY_MS);

    /* Read and validate calibration */
    int ret = bmp280_read_calibration(dev);
    if (ret != 0) {
        delay_ms(BMP280_RESET_DELAY_MS);
        ret = bmp280_read_calibration(dev);
        if (ret != 0)
            return -5;
    }

    /* Apply settings */
    if (bmp280_set_settings(dev, dev->settings.osr_t, dev->settings.osr_p, dev->settings.filter) != 0)
        return -6;

    /* Set mode */
    if (bmp280_set_mode(dev, dev->settings.mode) != 0)
        return -7;

    dev->ready = true;
    return 0;
}

/* ============================================================================
 * SETTINGS / MODE (VEGA SDK Pattern)
 * ========================================================================== */

int bmp280_set_settings(bmp280_dev_t *dev, uint8_t osr_t, uint8_t osr_p, uint8_t filter)
{
    if (dev == NULL)
        return -1;

    uint8_t ctrl_meas = (osr_t << 5) | (osr_p << 2) | dev->settings.mode;
    uint8_t config = (filter << 2) | (dev->settings.standby << 5);

    if (bmp280_write_reg(BMP280_REG_CTRL_MEAS, ctrl_meas) != 0)
        return -1;

    if (bmp280_write_reg(BMP280_REG_CONFIG, config) != 0)
        return -1;

    dev->settings.osr_t = osr_t;
    dev->settings.osr_p = osr_p;
    dev->settings.filter = filter;

    return 0;
}

int bmp280_set_mode(bmp280_dev_t *dev, uint8_t mode)
{
    if (dev == NULL)
        return -1;

    uint8_t ctrl_meas = (dev->settings.osr_t << 5) | (dev->settings.osr_p << 2) | mode;

    if (bmp280_write_reg(BMP280_REG_CTRL_MEAS, ctrl_meas) != 0)
        return -1;

    dev->settings.mode = mode;
    return 0;
}

/* ============================================================================
 * RAW DATA ACQUISITION
 * ========================================================================== */

int bmp280_read_raw(bmp280_dev_t *dev, bmp280_uncomp_t *uncomp)
{
    uint8_t status;
    uint8_t pbuf[3], tbuf[3];
    uint32_t timeout = BMP280_POLL_TIMEOUT_MS;

    if (!dev->ready || uncomp == NULL)
        return -1;

    /* Trigger forced measurement */
    if (bmp280_write_reg(BMP280_REG_CTRL_MEAS,
            (dev->settings.osr_t << 5) | (dev->settings.osr_p << 2) | BMP280_MODE_FORCED) != 0)
        return -1;

    delay_ms(BMP280_MEASURE_DELAY_MS);

    /* Wait for measurement complete */
    for (;;) {
        if (bmp280_read_regs(BMP280_REG_STATUS, &status, 1) != 0)
            return -1;

        if ((status & BMP280_STATUS_MEASURING) == 0)
            break;

        if (timeout < BMP280_POLL_PERIOD_MS)
            return -2;

        delay_ms(BMP280_POLL_PERIOD_MS);
        timeout -= BMP280_POLL_PERIOD_MS;
    }

    /* Read pressure */
    if (bmp280_read_regs(BMP280_REG_PRESS_MSB, pbuf, 3) != 0)
        return -1;

    /* Read temperature */
    if (bmp280_read_regs(BMP280_REG_TEMP_MSB, tbuf, 3) != 0)
        return -1;

    uncomp->pressure = ((uint32_t)pbuf[0] << 12) | ((uint32_t)pbuf[1] << 4) | (pbuf[2] >> 4);
    uncomp->temperature = ((uint32_t)tbuf[0] << 12) | ((uint32_t)tbuf[1] << 4) | (tbuf[2] >> 4);

    /* Validate */
    if (uncomp->pressure == 0 || uncomp->pressure == 0xFFFFF ||
        uncomp->temperature == 0 || uncomp->temperature == 0xFFFFF)
        return -3;

    return 0;
}

/* ============================================================================
 * COMPENSATION ALGORITHMS (Bosch BME280/BMP280 Official Algorithm)
 * ========================================================================== */

static int bmp280_compensate_temperature(bmp280_dev_t *dev, uint32_t adc_T, float *temp_c)
{
    int64_t var1, var2, tf;

    var1 = (((int64_t)(adc_T >> 3) - ((int64_t)dev->calib.dig_t1 << 1)) * (int64_t)dev->calib.dig_t2) >> 11;
    var2 = (((((int64_t)(adc_T >> 4) - (int64_t)dev->calib.dig_t1) *
             ((int64_t)(adc_T >> 4) - (int64_t)dev->calib.dig_t1)) >> 12) *
            (int64_t)dev->calib.dig_t3) >> 14;

    tf = var1 + var2;

    if (tf > INT32_MAX) tf = INT32_MAX;
    else if (tf < INT32_MIN) tf = INT32_MIN;

    dev->t_fine = (int32_t)tf;

    int64_t temp = (tf * 5 + 128) >> 8;
    if (temp > INT32_MAX) temp = INT32_MAX;
    else if (temp < INT32_MIN) temp = INT32_MIN;

    dev->last_temp_centi = (int32_t)temp;
    *temp_c = (float)temp / 100.0f;

    return 0;
}

static int bmp280_compensate_pressure(bmp280_dev_t *dev, uint32_t adc_P, float *press_pa)
{
    int32_t var1, var2;
    uint32_t p;

    var1 = (dev->t_fine / 2) - 64000;
    var2 = (((var1 / 4) * (var1 / 4)) / 2048) * (int32_t)dev->calib.dig_p6;
    var2 = var2 + ((var1 * (int32_t)dev->calib.dig_p5) * 2);
    var2 = (var2 / 4) + ((int32_t)dev->calib.dig_p4 * 65536);
    var1 = ((((int32_t)dev->calib.dig_p3 * (((var1 / 4) * (var1 / 4)) / 8192)) / 8) +
            (((int32_t)dev->calib.dig_p2 * var1) / 2)) / 262144;
    var1 = (((32768 + var1)) * (int32_t)dev->calib.dig_p1) / 32768;

    if (var1 == 0)
        return -1;

    int64_t p_inter = ((int64_t)(1048576 - adc_P) - (var2 / 4096)) * 3125;

    if (p_inter < 0)
        return -1;

    if ((uint64_t)p_inter < 0x80000000ULL)
        p = (uint32_t)((p_inter << 1) / (uint32_t)var1);
    else
        p = (uint32_t)((p_inter / (uint32_t)var1) * 2);

    int32_t var1_p = ((int32_t)dev->calib.dig_p9 * (int32_t)(((p / 8) * (p / 8)) / 8192)) / 4096;
    int32_t var2_p = ((int32_t)(p / 4) * (int32_t)dev->calib.dig_p8) / 8192;

    p = (uint32_t)((int32_t)p + ((var1_p + var2_p + dev->calib.dig_p7) / 16));

    dev->last_pressure_pa = p;
    *press_pa = (float)p;

    return 0;
}

int bmp280_compensate(bmp280_dev_t *dev, const bmp280_uncomp_t *uncomp, bmp280_data_t *data)
{
    float temp_c, press_pa;

    if (bmp280_compensate_temperature(dev, uncomp->temperature, &temp_c) != 0)
        return -1;

    if (bmp280_compensate_pressure(dev, uncomp->pressure, &press_pa) != 0)
        return -1;

    data->temperature = temp_c;
    data->pressure = press_pa;

    return 0;
}

/* ============================================================================
 * HIGH-LEVEL MEASUREMENT (VEGA SDK Pattern: measure + read)
 * ========================================================================== */

int bmp280_measure(bmp280_dev_t *dev, bmp280_data_t *data)
{
    bmp280_uncomp_t uncomp;

    if (!dev->ready || data == NULL)
        return -1;

    if (bmp280_read_raw(dev, &uncomp) != 0)
        return -1;

    if (bmp280_compensate(dev, &uncomp, data) != 0)
        return -1;

    /* Publish to barometer interface */
    barometer_data_t baro_data = {
        .pressure_pa = data->pressure,
        .temperature_c = data->temperature,
        .timestamp_ms = 0,
        .valid = true
    };
    barometer_publish(&baro_data);

    return 0;
}

/* ============================================================================
 * PUBLIC API (VEGA SDK Pattern)
 * ========================================================================== */

int bmp280_begin(bmp280_dev_t *dev, uint8_t osr_t, uint8_t osr_p, uint8_t filter, uint8_t mode)
{
    if (dev == NULL)
        return -1;

    dev->settings.osr_t = osr_t;
    dev->settings.osr_p = osr_p;
    dev->settings.filter = filter;
    dev->settings.mode = mode;
    dev->settings.standby = BMP280_STANDBY_0_5;

    return bmp280_init_device(dev);
}

int bmp280_read_calibration(bmp280_dev_t *dev)
{
    return bmp280_read_calibration(dev);
}

void bmp280_busy_wait(void)
{
    /* Not needed with our HAL - SPI is synchronous */
}