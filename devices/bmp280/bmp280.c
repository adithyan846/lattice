
/***********************************************************************************
 * FILE ID         : bmp280.c
 * VERSION         : 1.0.0
 * PROJECT         : LATTICE - ARIES V3
 * DESCRIPTION     : BMP280 SPI device driver for THEJAS32
 *
 * Responsibilities:
 *   - BMP280 initialisation and configuration
 *   - SPI register communication through HAL
 *   - Calibration coefficient acquisition
 *   - Raw pressure and temperature acquisition
 *   - BMP280 calibration compensation
 *   - Publishing measurements through barometer interface
 *
 * NOTE:
 *   SPI peripheral configuration must be performed by the platform layer
 *   before calling bmp280_init().
 ***********************************************************************************/

#include "bmp280.h"
#include "barometer.h"

#include "hal_timer.h"
#include "hal_spi.h"

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

/* ============================================================================
 * BMP280 REGISTER DEFINITIONS
 * ========================================================================== */

#define BMP280_CHIP_ID              0x58

#define BMP280_REG_ID               0xD0
#define BMP280_REG_RESET            0xE0
#define BMP280_RESET_VALUE          0xB6
#define BMP280_REG_STATUS           0xF3
#define BMP280_REG_CTRL_MEAS        0xF4
#define BMP280_REG_CONFIG           0xF5
#define BMP280_REG_CALIB            0x88
#define BMP280_REG_PRESS_MSB        0xF7
#define BMP280_REG_TEMP_MSB         0xFA

#define BMP280_STATUS_MEASURING     (1U << 3)

#define BMP280_SPI_READ             0x80
#define BMP280_SPI_WRITE_MASK       0x7F

/* ============================================================================
 * TIMING CONFIGURATION
 * ========================================================================== */

#define BMP280_RESET_DELAY_MS       10
#define BMP280_MEASURE_DELAY_MS     10
#define BMP280_POLL_TIMEOUT_MS      50
#define BMP280_POLL_PERIOD_MS       2

/* ============================================================================
 * SENSOR CONFIGURATION
 * ========================================================================== */

#define BMP280_OSRS_T               0x02
#define BMP280_OSRS_P               0x05
#define BMP280_IIR_FILTER           0x04

#define BMP280_CTRL_MEAS_FORCED \
    ((BMP280_OSRS_T << 5) | (BMP280_OSRS_P << 2) | 0x01)

#define BMP280_CTRL_MEAS_SLEEP \
    ((BMP280_OSRS_T << 5) | (BMP280_OSRS_P << 2) | 0x00)

#define BMP280_CONFIG_VALUE \
    (BMP280_IIR_FILTER << 2)

/* ============================================================================
 * SPI CONFIGURATION
 *
 * SPI peripheral configuration is owned by the platform layer.
 * These definitions identify the SPI peripheral used by this device.
 * ========================================================================== */

#define BARO_SPI_BUS                HAL_SPI_BUS_2
#define BARO_TIMEOUT_MS             100

/* ============================================================================
 * DEVICE DATA STRUCTURES
 * ========================================================================== */

/*
 * BMP280 factory calibration coefficients.
 *
 * These values are read from the sensor's calibration registers.
 */

typedef struct
{
    uint16_t T1;
    int16_t  T2;
    int16_t  T3;

    uint16_t P1;
    int16_t  P2;
    int16_t  P3;
    int16_t  P4;
    int16_t  P5;
    int16_t  P6;
    int16_t  P7;
    int16_t  P8;
    int16_t  P9;

} bmp280_calib_t;


/*
 * Internal BMP280 device state.
 *
 * This structure is private to the device driver.
 * Higher-level software must not depend on it.
 */

typedef struct
{
    int32_t t_fine;

    int32_t last_temp_centi;
    uint32_t last_pressure_pa;

    bmp280_calib_t calib;

} bmp280_dev_t;


/* ============================================================================
 * INTERNAL DEVICE STATE
 * ========================================================================== */

static bmp280_dev_t bmp280_device;

static bool bmp280_ready = false;


/* ============================================================================
 * BASIC HELPERS
 * ========================================================================== */

static void delay_ms(uint32_t ms)
{
    hal_timer_delay_ms(ms);
}


/* ============================================================================
 * SPI REGISTER OPERATIONS
 * ========================================================================== */

/*
 * Read one or more BMP280 registers.
 *
 * The BMP280 SPI read command requires bit 7 to be set.
 */

static int bmp280_read_regs(uint8_t reg, uint8_t *buf, uint8_t len)
{
    if (buf == NULL || len == 0)
        return -1;

    uint8_t cmd = reg | BMP280_SPI_READ;

    return hal_spi_write_read(
        BARO_SPI_BUS,
        &cmd,
        1,
        buf,
        len,
        BARO_TIMEOUT_MS
    );
}


/*
 * Write a value to a BMP280 register.
 *
 * The BMP280 SPI write command requires bit 7 to be cleared.
 */

static int bmp280_write_reg(uint8_t reg, uint8_t value)
{
    uint8_t tx[2] =
    {
        reg & BMP280_SPI_WRITE_MASK,
        value
    };

    return hal_spi_write(
        BARO_SPI_BUS,
        tx,
        2,
        BARO_TIMEOUT_MS
    );
}


/* ============================================================================
 * CALIBRATION DATA DECODING
 * ========================================================================== */

/*
 * Convert little-endian unsigned 16-bit data.
 */

static uint16_t le16u(const uint8_t *p)
{
    return (uint16_t)p[0] |
           ((uint16_t)p[1] << 8);
}


/*
 * Convert little-endian signed 16-bit data.
 */

static int16_t le16s(const uint8_t *p)
{
    return (int16_t)le16u(p);
}


/*
 * Load calibration coefficients from the 24-byte calibration block.
 */

static void bmp280_decode_calibration(
    bmp280_calib_t *calib,
    const uint8_t *data)
{
    calib->T1 = le16u(&data[0]);
    calib->T2 = le16s(&data[2]);
    calib->T3 = le16s(&data[4]);

    calib->P1 = le16u(&data[6]);
    calib->P2 = le16s(&data[8]);
    calib->P3 = le16s(&data[10]);
    calib->P4 = le16s(&data[12]);
    calib->P5 = le16s(&data[14]);
    calib->P6 = le16s(&data[16]);
    calib->P7 = le16s(&data[18]);
    calib->P8 = le16s(&data[20]);
    calib->P9 = le16s(&data[22]);
}


/*
 * Validate calibration coefficients.
 *
 * T1 and P1 must be non-zero and not erased.
 */

static int bmp280_calib_valid(const bmp280_calib_t *c)
{
    if (c->T1 == 0 || c->P1 == 0)
        return 0;

    if (c->T1 == 0xFFFF || c->P1 == 0xFFFF)
        return 0;

    if (c->T2 == 0 && c->T3 == 0)
        return 0;

    return 1;
}


/*
 * Read all 24 calibration bytes.
 */

static int bmp280_read_calibration(bmp280_dev_t *dev)
{
    uint8_t calib_data[24];

    if (bmp280_read_regs(
            BMP280_REG_CALIB,
            calib_data,
            sizeof(calib_data)) != 0)
    {
        return -1;
    }

    bmp280_decode_calibration(
        &dev->calib,
        calib_data
    );

    if (!bmp280_calib_valid(&dev->calib))
        return -2;

    return 0;
}


/* ============================================================================
 * DEVICE INITIALISATION
 * ========================================================================== */

/*
 * Internal initialisation.
 *
 * Sequence:
 *   1. Reset sensor.
 *   2. Verify chip ID.
 *   3. Read calibration coefficients.
 *   4. Validate calibration.
 *   5. Configure IIR filter and oversampling.
 */

static int bmp280_init_device(bmp280_dev_t *dev)
{
    uint8_t id;

    if (dev == NULL)
        return -1;

    /* Ensure software state starts clean. */
    dev->t_fine = 0;
    dev->last_temp_centi = 0;
    dev->last_pressure_pa = 0;

    delay_ms(BMP280_RESET_DELAY_MS);

    /* Verify BMP280 chip ID. */
    if (bmp280_read_regs(BMP280_REG_ID, &id, 1) != 0)
        return -2;

    if (id != BMP280_CHIP_ID)
        return -3;

    /* Reset BMP280. */
    if (bmp280_write_reg(
            BMP280_REG_RESET,
            BMP280_RESET_VALUE) != 0)
    {
        return -4;
    }

    delay_ms(BMP280_RESET_DELAY_MS);

    /* Read and validate calibration data. */
    int ret = bmp280_read_calibration(dev);

    /*
     * Retry once if calibration data is invalid.
     */
    if (ret != 0)
    {
        delay_ms(BMP280_RESET_DELAY_MS);

        ret = bmp280_read_calibration(dev);

        if (ret != 0)
            return -5;
    }

    /* Configure IIR filter. */
    if (bmp280_write_reg(
            BMP280_REG_CONFIG,
            BMP280_CONFIG_VALUE) != 0)
    {
        return -6;
    }

    /* Configure oversampling and leave sensor in sleep mode. */
    if (bmp280_write_reg(
            BMP280_REG_CTRL_MEAS,
            BMP280_CTRL_MEAS_SLEEP) != 0)
    {
        return -7;
    }

    return 0;
}


/* ============================================================================
 * RAW SENSOR DATA ACQUISITION
 * ========================================================================== */

/*
 * Read raw pressure and temperature ADC values.
 *
 * Returns:
 *   0  - Success
 *  -1  - Communication error
 *  -2  - Measurement timeout
 *  -3  - Invalid ADC values
 */

static int bmp280_read_raw(
    int32_t *adc_P,
    int32_t *adc_T)
{
    uint8_t status;
    uint8_t pbuf[3];
    uint8_t tbuf[3];

    uint32_t timeout = BMP280_POLL_TIMEOUT_MS;

    if (adc_P == NULL || adc_T == NULL)
        return -1;

    /* Start forced-mode measurement. */
    if (bmp280_write_reg(
            BMP280_REG_CTRL_MEAS,
            BMP280_CTRL_MEAS_FORCED) != 0)
    {
        return -1;
    }

    delay_ms(BMP280_MEASURE_DELAY_MS);

    /* Wait until measurement is complete. */
    for (;;)
    {
        if (bmp280_read_regs(
                BMP280_REG_STATUS,
                &status,
                1) != 0)
        {
            return -1;
        }

        if ((status & BMP280_STATUS_MEASURING) == 0)
            break;

        if (timeout < BMP280_POLL_PERIOD_MS)
            return -2;

        delay_ms(BMP280_POLL_PERIOD_MS);

        timeout -= BMP280_POLL_PERIOD_MS;
    }

    /* Read pressure ADC registers. */
    if (bmp280_read_regs(
            BMP280_REG_PRESS_MSB,
            pbuf,
            3) != 0)
    {
        return -1;
    }

    /* Read temperature ADC registers. */
    if (bmp280_read_regs(
            BMP280_REG_TEMP_MSB,
            tbuf,
            3) != 0)
    {
        return -1;
    }

    /* Reconstruct 20-bit ADC values. */
    *adc_P =
        ((int32_t)pbuf[0] << 12) |
        ((int32_t)pbuf[1] << 4) |
        ((int32_t)pbuf[2] >> 4);

    *adc_T =
        ((int32_t)tbuf[0] << 12) |
        ((int32_t)tbuf[1] << 4) |
        ((int32_t)tbuf[2] >> 4);

    /* Reject invalid ADC readings. */
    if (*adc_P == 0 || *adc_P == 0xFFFFF ||
        *adc_T == 0 || *adc_T == 0xFFFFF)
    {
        return -3;
    }

    return 0;
}


/* ============================================================================
 * BMP280 CALIBRATION COMPENSATION
 * ========================================================================== */

/*
 * Convert raw ADC temperature and pressure readings into:
 *
 *   temperature_c : degrees Celsius
 *   pressure_pa   : Pascals
 *
 * Uses the BMP280 integer compensation algorithm.
 */

static int bmp280_compensate(
    bmp280_dev_t *dev,
    int32_t adc_P,
    int32_t adc_T,
    float *temperature_c,
    float *pressure_pa)
{
    int64_t v1_64;
    int64_t v2_64;
    int64_t tf_64;
    int64_t temp_64;

    int32_t var1;
    int32_t var2;
    int32_t temp;

    uint32_t p;

    if (dev == NULL)
        return -1;

    /* ---------------------------------------------------------------------
     * TEMPERATURE COMPENSATION
     * ------------------------------------------------------------------ */

    v1_64 =
        (((int64_t)(adc_T >> 3) -
          ((int64_t)dev->calib.T1 << 1)) *
          (int64_t)dev->calib.T2) >> 11;

    v2_64 =
        (((((int64_t)(adc_T >> 4) -
             (int64_t)dev->calib.T1) *
            ((int64_t)(adc_T >> 4) -
             (int64_t)dev->calib.T1)) >> 12) *
          (int64_t)dev->calib.T3) >> 14;

    tf_64 = v1_64 + v2_64;

    if (tf_64 > INT32_MAX)
        tf_64 = INT32_MAX;
    else if (tf_64 < INT32_MIN)
        tf_64 = INT32_MIN;

    dev->t_fine = (int32_t)tf_64;

    temp_64 = (tf_64 * 5 + 128) >> 8;

    if (temp_64 > INT32_MAX)
        temp_64 = INT32_MAX;
    else if (temp_64 < INT32_MIN)
        temp_64 = INT32_MIN;

    temp = (int32_t)temp_64;

    dev->last_temp_centi = temp;

    if (temperature_c != NULL)
        *temperature_c = (float)temp / 100.0f;


    /* ---------------------------------------------------------------------
     * PRESSURE COMPENSATION
     * ------------------------------------------------------------------ */

    var1 = (dev->t_fine / 2) - 64000;

    var2 =
        (((var1 / 4) * (var1 / 4)) / 2048) *
        (int32_t)dev->calib.P6;

    var2 =
        var2 +
        ((var1 * (int32_t)dev->calib.P5) * 2);

    var2 =
        (var2 / 4) +
        ((int32_t)dev->calib.P4 * 65536);

    var1 =
        (((int32_t)dev->calib.P3 *
          (((var1 / 4) * (var1 / 4)) / 8192)) / 8 +
         (((int32_t)dev->calib.P2 * var1) / 2)) / 262144;

    var1 =
        ((32768 + var1) *
         (int32_t)dev->calib.P1) / 32768;

    if (var1 == 0)
    {
        return -2;
    }

    /*
     * Pressure intermediate calculation.
     *
     * Use a 64-bit intermediate to avoid signed 32-bit
     * overflow during multiplication.
     */

    int64_t pressure_intermediate =
        ((int64_t)(1048576 - adc_P) -
         ((int64_t)var2 / 4096)) * 3125;

    if (pressure_intermediate < 0)
        return -2;

    if ((uint64_t)pressure_intermediate < 0x80000000ULL)
    {
        p = ((uint32_t)pressure_intermediate << 1) /
            (uint32_t)var1;
    }
    else
    {
        p = (uint32_t)pressure_intermediate /
            (uint32_t)var1;

        p *= 2U;
    }

    var1 =
        ((int32_t)dev->calib.P9 *
         (int32_t)(((p / 8U) * (p / 8U)) / 8192U)) / 4096;

    var2 =
        ((int32_t)(p / 4U) *
         (int32_t)dev->calib.P8) / 8192;

    p =
        (uint32_t)((int32_t)p +
        ((var1 + var2 + dev->calib.P7) / 16));

    dev->last_pressure_pa = p;

    if (pressure_pa != NULL)
        *pressure_pa = (float)p;

    return 0;
}


/* ============================================================================
 * INTERNAL MEASUREMENT FUNCTION
 * ========================================================================== */

/*
 * Complete measurement cycle:
 *
 *   1. Trigger measurement.
 *   2. Read raw ADC values.
 *   3. Apply calibration compensation.
 */

static int bmp280_read_measurement(
    bmp280_dev_t *dev,
    float *temperature_c,
    float *pressure_pa)
{
    int32_t adc_P;
    int32_t adc_T;

    if (dev == NULL)
        return -1;

    int ret = bmp280_read_raw(&adc_P, &adc_T);

    if (ret != 0)
        return ret;

    return bmp280_compensate(
        dev,
        adc_P,
        adc_T,
        temperature_c,
        pressure_pa
    );
}


/* ============================================================================
 * PUBLIC DEVICE API
 * ========================================================================== */

/*
 * Initialise BMP280.
 *
 * Prerequisite:
 *   SPI2 must already be configured by the platform layer.
 */

int bmp280_init(void)
{
    bmp280_ready = false;

    int ret = bmp280_init_device(&bmp280_device);

    if (ret != 0)
        return ret;

    bmp280_ready = true;

    return 0;
}


/*
 * Perform one measurement and publish it through the
 * barometer interface.
 */

int bmp280_update(void)
{
    if (!bmp280_ready)
        return -1;

    barometer_data_t data = {0};

    int ret = bmp280_read_measurement(
        &bmp280_device,
        &data.temperature_c,
        &data.pressure_pa
    );

    if (ret != 0)
        return ret;

    /*
     * TODO:
     * Replace with the Lattice system timestamp API.
     */
    data.timestamp_ms = 0;

    data.valid = true;

    /*
     * Publish calibrated measurements.
     * The application accesses them through barometer_get_latest().
     */
    barometer_publish(&data);

    return 0;
}

