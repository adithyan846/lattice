/***********************************************************************************
 * FILE ID         : lsm6dso.c
 * VERSION         : 1.0.0
 * PROJECT         : LATTICE - ARIES V3
 * DESCRIPTION     : LSM6DSO accelerometer / gyroscope SPI device driver
 *
 * Responsibilities:
 *   - LSM6DSO identification, reset and configuration
 *   - SPI register communication through HAL
 *   - Single 12-byte burst read of gyro + accel output registers
 *   - Conversion to milli-dps and milli-g (integer only)
 *   - Publishing measurements through the IMU interface
 *
 * Only the registers needed for accel/gyro sampling are used.  FIFO, sensor
 * hub, FSM, pedometer, tap, OIS, interrupt routing and self-test are omitted.
 *
 * Ported from the ARIES v3 bring-up driver (~/drivers/test-lattice/gyro).
 *
 * NOTE:
 *   SPI peripheral configuration must be performed by the platform layer
 *   before calling lsm6dso_init().
 ***********************************************************************************/

#include "lsm6dso.h"
#include "imu.h"

#include "hal_timer.h"
#include "hal_spi.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ============================================================================
 * LSM6DSO REGISTER DEFINITIONS (main page, only what this driver uses)
 * ========================================================================== */

#define LSM6DSO_REG_WHO_AM_I        0x0FU
#define LSM6DSO_REG_CTRL1_XL        0x10U
#define LSM6DSO_REG_CTRL2_G         0x11U
#define LSM6DSO_REG_CTRL3_C         0x12U
#define LSM6DSO_REG_CTRL4_C         0x13U
#define LSM6DSO_REG_STATUS          0x1EU
#define LSM6DSO_REG_OUTX_L_G        0x22U   /* gyro XYZ, then accel XYZ */

#define LSM6DSO_WHO_AM_I_VALUE      0x6CU

#define LSM6DSO_SPI_READ            0x80U
#define LSM6DSO_SPI_WRITE_MASK      0x7FU

#define LSM6DSO_STATUS_XLDA         0x01U   /* accel new data available */
#define LSM6DSO_STATUS_GDA          0x02U   /* gyro new data available  */

#define LSM6DSO_CTRL3_SW_RESET      0x01U
#define LSM6DSO_CTRL3_IF_INC        0x04U   /* auto-increment for bursts */
#define LSM6DSO_CTRL3_BDU           0x40U   /* block data update         */

#define LSM6DSO_CTRL4_I2C_DISABLE   0x04U   /* SPI only, ignore I2C/I3C  */

/* ============================================================================
 * TIMING CONFIGURATION
 * ========================================================================== */

#define LSM6DSO_RESET_DELAY_MS      10
#define LSM6DSO_SETTLE_DELAY_MS     50      /* digital filters settle */

/* ============================================================================
 * SENSOR CONFIGURATION
 *
 * 1.666 kHz, +/-16 g, +/-2000 dps.  High-performance mode is the reset
 * default in CTRL6_C/CTRL7_G, and CTRL8_XL.XL_FS_MODE resets to 0 so
 * FS_XL = 01 really selects +/-16 g.
 * ========================================================================== */

#define LSM6DSO_ODR_1666HZ          0x08U
#define LSM6DSO_XL_FS_16G           0x01U
#define LSM6DSO_G_FS_2000DPS        0x03U

/* CTRL1_XL: ODR at bits 7:4, FS_XL at bits 3:2.  CTRL2_G likewise. */
#define LSM6DSO_CTRL1_XL_VALUE \
    ((uint8_t)((LSM6DSO_ODR_1666HZ << 4) | (LSM6DSO_XL_FS_16G << 2)))

#define LSM6DSO_CTRL2_G_VALUE \
    ((uint8_t)((LSM6DSO_ODR_1666HZ << 4) | (LSM6DSO_G_FS_2000DPS << 2)))

/*
 * Sensitivities from the datasheet, as exact integer fractions so no
 * soft-float is needed.  These MUST match the full-scale settings above; the
 * unused ranges are kept so changing FS is a one-line edit.
 *
 *   gyro  : mdps/LSB = NUM / DEN  (4.375, 8.75, 17.5, 35, 70)
 *   accel : ug/LSB                (61, 122, 244, 488)
 *
 * Largest intermediate: 32768 * 488 = 16e6, well inside int32_t.
 */
#define LSM6DSO_GYRO_MDPS_NUM_125DPS    35
#define LSM6DSO_GYRO_MDPS_DEN_125DPS    8
#define LSM6DSO_GYRO_MDPS_NUM_250DPS    35
#define LSM6DSO_GYRO_MDPS_DEN_250DPS    4
#define LSM6DSO_GYRO_MDPS_NUM_500DPS    35
#define LSM6DSO_GYRO_MDPS_DEN_500DPS    2
#define LSM6DSO_GYRO_MDPS_NUM_1000DPS   35
#define LSM6DSO_GYRO_MDPS_DEN_1000DPS   1
#define LSM6DSO_GYRO_MDPS_NUM_2000DPS   70
#define LSM6DSO_GYRO_MDPS_DEN_2000DPS   1

#define LSM6DSO_ACCEL_UG_2G             61
#define LSM6DSO_ACCEL_UG_4G             122
#define LSM6DSO_ACCEL_UG_8G             244
#define LSM6DSO_ACCEL_UG_16G            488

#define LSM6DSO_GYRO_MDPS_NUM           LSM6DSO_GYRO_MDPS_NUM_2000DPS
#define LSM6DSO_GYRO_MDPS_DEN           LSM6DSO_GYRO_MDPS_DEN_2000DPS
#define LSM6DSO_ACCEL_UG_PER_LSB        LSM6DSO_ACCEL_UG_16G

/* Gyro XYZ followed by accel XYZ = 12 bytes starting at OUTX_L_G. */
#define LSM6DSO_DATA_BURST_LEN      12U

#define LSM6DSO_SPI_TIMEOUT_MS      10

/* ============================================================================
 * DEVICE DATA STRUCTURES
 * ========================================================================== */

/*
 * Internal LSM6DSO device state.
 *
 * This structure is private to the device driver.
 * Higher-level software must not depend on it.
 */

typedef struct
{
    hal_spi_bus_t bus;
    uint8_t who_am_i;
} lsm6dso_dev_t;


/* ============================================================================
 * INTERNAL DEVICE STATE
 * ========================================================================== */

static lsm6dso_dev_t lsm6dso_device;

static bool lsm6dso_ready = false;


/* ============================================================================
 * BASIC HELPERS
 * ========================================================================== */

static void delay_ms(uint32_t ms)
{
    hal_timer_delay_ms(ms);
}


/* Little-endian pair from the LSM6DSO output registers. */
static int16_t le16s(const uint8_t *p)
{
    return (int16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}


/* ============================================================================
 * SPI REGISTER OPERATIONS
 * ========================================================================== */

/*
 * Read one or more LSM6DSO registers.
 *
 * The LSM6DSO SPI read command requires bit 7 to be set.  Multi-byte reads
 * rely on CTRL3_C.IF_INC, which is set at reset and kept by lsm6dso_init().
 */

static int lsm6dso_read_regs(lsm6dso_dev_t *dev, uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t cmd = (uint8_t)(reg | LSM6DSO_SPI_READ);

    if (hal_spi_write_read(
            dev->bus,
            &cmd,
            1,
            buf,
            len,
            LSM6DSO_SPI_TIMEOUT_MS) != 0)
    {
        return LSM6DSO_ERR_BUS;
    }

    return LSM6DSO_OK;
}


/*
 * Write a value to an LSM6DSO register.
 *
 * The LSM6DSO SPI write command requires bit 7 to be cleared.
 */

static int lsm6dso_write_reg(lsm6dso_dev_t *dev, uint8_t reg, uint8_t value)
{
    uint8_t tx[2] =
    {
        (uint8_t)(reg & LSM6DSO_SPI_WRITE_MASK),
        value
    };

    if (hal_spi_write(
            dev->bus,
            tx,
            2,
            LSM6DSO_SPI_TIMEOUT_MS) != 0)
    {
        return LSM6DSO_ERR_BUS;
    }

    return LSM6DSO_OK;
}


/* ============================================================================
 * DEVICE INITIALISATION
 * ========================================================================== */

/*
 * Internal initialisation.
 *
 * Sequence:
 *   1. Verify WHO_AM_I before changing any configuration.
 *   2. Soft reset.
 *   3. Disable the I2C interface.
 *   4. Enable block data update and register auto-increment.
 *   5. Configure accelerometer and gyroscope.
 *   6. Wait for the digital filters to settle.
 *
 * I2C is enabled at power-up and shares pins with SPI (SDA = SDI,
 * SCL = SPC).  Disabling it is a precaution for SPI-only use so line
 * activity can never be taken as I2C traffic.  It is written after
 * SW_RESET, which clears it.
 */

static int lsm6dso_init_device(lsm6dso_dev_t *dev, hal_spi_bus_t bus)
{
    int ret;

    dev->bus = bus;
    dev->who_am_i = 0;

    ret = lsm6dso_read_regs(dev, LSM6DSO_REG_WHO_AM_I, &dev->who_am_i, 1);
    if (ret != LSM6DSO_OK)
        return ret;

    if (dev->who_am_i != LSM6DSO_WHO_AM_I_VALUE)
        return LSM6DSO_ERR_WHO_AM_I;

    /* Soft reset; the reset bit self-clears after the device boot time. */
    ret = lsm6dso_write_reg(dev, LSM6DSO_REG_CTRL3_C, LSM6DSO_CTRL3_SW_RESET);
    if (ret != LSM6DSO_OK)
        return ret;

    delay_ms(LSM6DSO_RESET_DELAY_MS);

    ret = lsm6dso_write_reg(dev, LSM6DSO_REG_CTRL4_C, LSM6DSO_CTRL4_I2C_DISABLE);
    if (ret != LSM6DSO_OK)
        return ret;

    /* Coherent 16-bit outputs and automatic address increment for bursts. */
    ret = lsm6dso_write_reg(dev, LSM6DSO_REG_CTRL3_C,
                            LSM6DSO_CTRL3_BDU | LSM6DSO_CTRL3_IF_INC);
    if (ret != LSM6DSO_OK)
        return ret;

    ret = lsm6dso_write_reg(dev, LSM6DSO_REG_CTRL1_XL, LSM6DSO_CTRL1_XL_VALUE);
    if (ret != LSM6DSO_OK)
        return ret;

    ret = lsm6dso_write_reg(dev, LSM6DSO_REG_CTRL2_G, LSM6DSO_CTRL2_G_VALUE);
    if (ret != LSM6DSO_OK)
        return ret;

    delay_ms(LSM6DSO_SETTLE_DELAY_MS);

    return LSM6DSO_OK;
}


/* ============================================================================
 * SENSOR DATA ACQUISITION
 * ========================================================================== */

/*
 * Returns 1 when both accel and gyro have new data, 0 when not,
 * negative on bus error.
 */

static int lsm6dso_data_ready(lsm6dso_dev_t *dev)
{
    uint8_t status;

    int ret = lsm6dso_read_regs(dev, LSM6DSO_REG_STATUS, &status, 1);
    if (ret != LSM6DSO_OK)
        return ret;

    return ((status & (LSM6DSO_STATUS_XLDA | LSM6DSO_STATUS_GDA)) ==
            (LSM6DSO_STATUS_XLDA | LSM6DSO_STATUS_GDA)) ? 1 : 0;
}


/*
 * One burst: gyro X/Y/Z then accel X/Y/Z, little-endian int16 each.
 * BDU guarantees the high and low bytes of each axis belong together.
 */

static int lsm6dso_read_sample(lsm6dso_dev_t *dev, imu_data_t *data)
{
    uint8_t raw[LSM6DSO_DATA_BURST_LEN];

    int ret = lsm6dso_read_regs(dev, LSM6DSO_REG_OUTX_L_G, raw, sizeof(raw));
    if (ret != LSM6DSO_OK)
        return ret;

    for (uint8_t axis = 0; axis < 3; axis++)
    {
        data->gyro_raw[axis] = le16s(&raw[axis * 2U]);
        data->accel_raw[axis] = le16s(&raw[6U + axis * 2U]);

        data->gyro_mdps[axis] =
            ((int32_t)data->gyro_raw[axis] * LSM6DSO_GYRO_MDPS_NUM) / LSM6DSO_GYRO_MDPS_DEN;
        data->accel_mg[axis] =
            ((int32_t)data->accel_raw[axis] * LSM6DSO_ACCEL_UG_PER_LSB) / 1000;
    }

    return LSM6DSO_OK;
}


/* ============================================================================
 * PUBLIC DEVICE API
 * ========================================================================== */

/*
 * Initialise LSM6DSO on the given SPI bus.
 *
 * Prerequisite:
 *   The bus must already be configured by the platform layer.
 */

int lsm6dso_init(hal_spi_bus_t bus)
{
    lsm6dso_ready = false;

    int ret = lsm6dso_init_device(&lsm6dso_device, bus);

    if (ret != LSM6DSO_OK)
        return ret;

    lsm6dso_ready = true;

    return LSM6DSO_OK;
}


/*
 * Read one sample if available and publish it through the IMU interface.
 */

int lsm6dso_update(void)
{
    if (!lsm6dso_ready)
        return LSM6DSO_ERR_NOT_READY;

    int ready = lsm6dso_data_ready(&lsm6dso_device);

    if (ready < 0)
        return ready;

    if (ready == 0)
        return LSM6DSO_NO_NEW_DATA;

    imu_data_t data = {0};

    int ret = lsm6dso_read_sample(&lsm6dso_device, &data);

    if (ret != LSM6DSO_OK)
        return ret;

    /*
     * TODO:
     * Replace with the Lattice system timestamp API.
     */
    data.timestamp_ms = 0;

    data.valid = true;

    imu_publish(&data);

    return LSM6DSO_OK;
}


uint8_t lsm6dso_last_who_am_i(void)
{
    return lsm6dso_device.who_am_i;
}
