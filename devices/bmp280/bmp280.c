/***********************************************************************************
 * FILE ID         : bmp280.c
 * VERSION         : 1.1.0
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
 * Hardware notes (from bring-up on ARIES v3, see ~/drivers/test-lattice/baro):
 *   - Some BMP280 modules do not return long auto-increment bursts correctly:
 *     the first bytes are fine and the rest read 0x00, which collapses every
 *     compensated value to zero.  Every register is therefore read with its
 *     own single-byte transaction.
 *   - Soft-float on this toolchain returns wrong, non-repeatable results and
 *     64-bit division (__divdi3) misbehaves.  The driver therefore uses
 *     integers only: 64-bit values are multiplied and shifted, never divided.
 *
 * Based on:
 *   ebrezadev/BMP280-Barometric-Pressure-and-Temperature-Sensor-C-Driver
 *   MIT License, Copyright (c) 2025 Reza G. Ebrahimi
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
#include <stddef.h>
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

#define BMP280_CALIB_LEN            24U

#define BMP280_STATUS_MEASURING     (1U << 3)

#define BMP280_SPI_READ             0x80
#define BMP280_SPI_WRITE_MASK       0x7F

/* ============================================================================
 * TIMING CONFIGURATION
 * ========================================================================== */

#define BMP280_RESET_DELAY_MS       10

/*
 * Wait for the conversion even if the status bit cannot be read correctly.
 * With temperature x2 and pressure x16 a forced conversion takes ~43 ms.
 */
#define BMP280_MEASURE_DELAY_MS     50
#define BMP280_POLL_TIMEOUT_MS      100
#define BMP280_POLL_PERIOD_MS       1

/* ============================================================================
 * SENSOR CONFIGURATION
 * ========================================================================== */

#define BMP280_OSRS_T               0x02    /* temperature oversampling x2  */
#define BMP280_OSRS_P               0x05    /* pressure oversampling x16    */
#define BMP280_IIR_FILTER           0x04    /* IIR filter x16               */

#define BMP280_CTRL_MEAS_FORCED \
    ((BMP280_OSRS_T << 5) | (BMP280_OSRS_P << 2) | 0x01)

#define BMP280_CTRL_MEAS_SLEEP \
    ((BMP280_OSRS_T << 5) | (BMP280_OSRS_P << 2) | 0x00)

#define BMP280_CONFIG_VALUE \
    (BMP280_IIR_FILTER << 2)

#define BMP280_SPI_TIMEOUT_MS       100

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
    hal_spi_bus_t bus;

    int32_t t_fine;

    int32_t last_temp_centi;
    uint32_t last_pressure_pa;

    uint8_t chip_id;

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
 * Read one BMP280 register.
 *
 * The BMP280 SPI read command requires bit 7 to be set.
 */

static int bmp280_read_reg(bmp280_dev_t *dev, uint8_t reg, uint8_t *value)
{
    uint8_t cmd = reg | BMP280_SPI_READ;

    if (hal_spi_write_read(
            dev->bus,
            &cmd,
            1,
            value,
            1,
            BMP280_SPI_TIMEOUT_MS) != 0)
    {
        return BMP280_ERR_BUS;
    }

    return BMP280_OK;
}


/*
 * Read consecutive BMP280 registers, one transaction per register.
 *
 * Deliberately avoids auto-increment bursts (see file header).
 */

static int bmp280_read_regs(bmp280_dev_t *dev, uint8_t reg, uint8_t *buf, uint8_t len)
{
    if (buf == NULL || len == 0)
        return BMP280_ERR_BUS;

    for (uint8_t i = 0; i < len; i++)
    {
        int ret = bmp280_read_reg(dev, (uint8_t)(reg + i), &buf[i]);

        if (ret != BMP280_OK)
            return ret;
    }

    return BMP280_OK;
}


/*
 * Write a value to a BMP280 register.
 *
 * The BMP280 SPI write command requires bit 7 to be cleared.
 */

static int bmp280_write_reg(bmp280_dev_t *dev, uint8_t reg, uint8_t value)
{
    uint8_t tx[2] =
    {
        reg & BMP280_SPI_WRITE_MASK,
        value
    };

    if (hal_spi_write(
            dev->bus,
            tx,
            2,
            BMP280_SPI_TIMEOUT_MS) != 0)
    {
        return BMP280_ERR_BUS;
    }

    return BMP280_OK;
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
 * A real BMP280 always has non-zero trim values.  All 0x00 (bus returned
 * nothing) or all 0xFF (MISO floating) means the NVM read failed.  A partly
 * read block (T1 valid, T2/T3 zero) collapses temperature to exactly 0.00 C.
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
    uint8_t calib_data[BMP280_CALIB_LEN];

    int ret = bmp280_read_regs(
        dev,
        BMP280_REG_CALIB,
        calib_data,
        sizeof(calib_data));

    if (ret != BMP280_OK)
        return ret;

    bmp280_decode_calibration(
        &dev->calib,
        calib_data
    );

    if (!bmp280_calib_valid(&dev->calib))
        return BMP280_ERR_CALIB;

    return BMP280_OK;
}


/* ============================================================================
 * DEVICE INITIALISATION
 * ========================================================================== */

/*
 * Internal initialisation.
 *
 * Sequence:
 *   1. Release CS and let the sensor power up.
 *   2. Verify chip ID.
 *   3. Soft reset.
 *   4. Read and validate calibration coefficients (retry once).
 *   5. Configure IIR filter and oversampling, stay in sleep mode.
 */

static int bmp280_init_device(bmp280_dev_t *dev, hal_spi_bus_t bus)
{
    int ret;

    /* Ensure software state starts clean. */
    dev->bus = bus;
    dev->t_fine = 0;
    dev->last_temp_centi = 0;
    dev->last_pressure_pa = 0;
    dev->chip_id = 0;

    /* Make sure CSB is idle high before the first transaction. */
    hal_spi_set_cs(bus, false);
    delay_ms(BMP280_RESET_DELAY_MS);

    /* Verify BMP280 chip ID. */
    ret = bmp280_read_reg(dev, BMP280_REG_ID, &dev->chip_id);
    if (ret != BMP280_OK)
        return ret;

    if (dev->chip_id != BMP280_CHIP_ID)
        return BMP280_ERR_CHIP_ID;

    /* Reset BMP280. */
    ret = bmp280_write_reg(dev, BMP280_REG_RESET, BMP280_RESET_VALUE);
    if (ret != BMP280_OK)
        return ret;

    delay_ms(BMP280_RESET_DELAY_MS);

    /*
     * The first read can come back empty if the reset has not fully
     * completed.  Retry once after the wake-up time before giving up.
     */
    ret = bmp280_read_calibration(dev);

    if (ret != BMP280_OK)
    {
        delay_ms(BMP280_RESET_DELAY_MS);

        ret = bmp280_read_calibration(dev);

        if (ret != BMP280_OK)
            return ret;
    }

    /* Configure IIR filter.  Standby time is irrelevant in forced mode. */
    ret = bmp280_write_reg(dev, BMP280_REG_CONFIG, BMP280_CONFIG_VALUE);
    if (ret != BMP280_OK)
        return ret;

    /* Configure oversampling and leave sensor in sleep mode. */
    return bmp280_write_reg(dev, BMP280_REG_CTRL_MEAS, BMP280_CTRL_MEAS_SLEEP);
}


/* ============================================================================
 * RAW SENSOR DATA ACQUISITION
 * ========================================================================== */

/*
 * Trigger a forced measurement and read raw pressure and temperature ADC
 * values.
 */

static int bmp280_read_raw(
    bmp280_dev_t *dev,
    int32_t *adc_P,
    int32_t *adc_T)
{
    uint8_t status;
    uint8_t pbuf[3];
    uint8_t tbuf[3];
    int ret;

    uint32_t timeout = BMP280_POLL_TIMEOUT_MS;

    /* Start forced-mode measurement. */
    ret = bmp280_write_reg(dev, BMP280_REG_CTRL_MEAS, BMP280_CTRL_MEAS_FORCED);
    if (ret != BMP280_OK)
        return ret;

    delay_ms(BMP280_MEASURE_DELAY_MS);

    /* Wait until measurement is complete. */
    for (;;)
    {
        ret = bmp280_read_reg(dev, BMP280_REG_STATUS, &status);
        if (ret != BMP280_OK)
            return ret;

        if ((status & BMP280_STATUS_MEASURING) == 0)
            break;

        if (timeout < BMP280_POLL_PERIOD_MS)
            return BMP280_ERR_TIMEOUT;

        delay_ms(BMP280_POLL_PERIOD_MS);

        timeout -= BMP280_POLL_PERIOD_MS;
    }

    /* Read pressure and temperature ADC registers. */
    ret = bmp280_read_regs(dev, BMP280_REG_PRESS_MSB, pbuf, 3);
    if (ret != BMP280_OK)
        return ret;

    ret = bmp280_read_regs(dev, BMP280_REG_TEMP_MSB, tbuf, 3);
    if (ret != BMP280_OK)
        return ret;

    /* Reconstruct 20-bit ADC values. */
    *adc_P =
        ((int32_t)pbuf[0] << 12) |
        ((int32_t)pbuf[1] << 4) |
        ((int32_t)pbuf[2] >> 4);

    *adc_T =
        ((int32_t)tbuf[0] << 12) |
        ((int32_t)tbuf[1] << 4) |
        ((int32_t)tbuf[2] >> 4);

    /*
     * A dead bus reads back all 0x00 or all 0xFF.  Reject only when both
     * raw values sit at an extreme; one extreme value alone can be real.
     */
    if ((*adc_P == 0 || *adc_P == 0xFFFFF) &&
        (*adc_T == 0 || *adc_T == 0xFFFFF))
    {
        return BMP280_ERR_DATA;
    }

    return BMP280_OK;
}


/* ============================================================================
 * BMP280 CALIBRATION COMPENSATION
 * ========================================================================== */

/*
 * Convert raw ADC readings into dev->last_temp_centi (0.01 C) and
 * dev->last_pressure_pa (Pa).
 *
 * Uses the BMP280 integer compensation algorithm.  64-bit values are only
 * multiplied and shifted, never divided (see file header).
 */

static int bmp280_compensate(
    bmp280_dev_t *dev,
    int32_t adc_P,
    int32_t adc_T)
{
    int64_t v1_64;
    int64_t v2_64;
    int64_t tf_64;
    int64_t temp_64;

    int32_t var1;
    int32_t var2;

    uint32_t p;

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

    dev->last_temp_centi = (int32_t)temp_64;


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
        return BMP280_ERR_COMPENSATE;
    }

    /*
     * Bosch reference arithmetic: the product may exceed INT32_MAX, so it is
     * done in uint32_t (defined wrap-around), and the branches below handle
     * values with the top bit set.
     */
    p = (uint32_t)((int32_t)(1048576 - adc_P) - (var2 / 4096)) * 3125U;

    if (p < 0x80000000U)
    {
        p = (p << 1) / (uint32_t)var1;
    }
    else
    {
        p = (p / (uint32_t)var1) * 2U;
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

    return BMP280_OK;
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

static int bmp280_read_measurement(bmp280_dev_t *dev)
{
    int32_t adc_P;
    int32_t adc_T;

    int ret = bmp280_read_raw(dev, &adc_P, &adc_T);

    if (ret != BMP280_OK)
        return ret;

    return bmp280_compensate(dev, adc_P, adc_T);
}


/* ============================================================================
 * PUBLIC DEVICE API
 * ========================================================================== */

/*
 * Initialise BMP280 on the given SPI bus.
 *
 * Prerequisite:
 *   The bus must already be configured by the platform layer.
 */

int bmp280_init(hal_spi_bus_t bus)
{
    bmp280_ready = false;

    int ret = bmp280_init_device(&bmp280_device, bus);

    if (ret != BMP280_OK)
        return ret;

    bmp280_ready = true;

    return BMP280_OK;
}


/*
 * Perform one measurement and publish it through the
 * barometer interface.
 */

int bmp280_update(void)
{
    if (!bmp280_ready)
        return BMP280_ERR_NOT_READY;

    int ret = bmp280_read_measurement(&bmp280_device);

    if (ret != BMP280_OK)
        return ret;

    barometer_data_t data = {0};

    data.temperature_centi_c = bmp280_device.last_temp_centi;
    data.pressure_pa = bmp280_device.last_pressure_pa;

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

    return BMP280_OK;
}


uint8_t bmp280_last_chip_id(void)
{
    return bmp280_device.chip_id;
}
