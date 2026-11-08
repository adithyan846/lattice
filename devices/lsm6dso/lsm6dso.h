#ifndef LATTICE_LSM6DSO_H
#define LATTICE_LSM6DSO_H

#include <stdint.h>
#include "hal_spi.h"

/* ============================================================================
 * RETURN CODES
 *
 * lsm6dso_update() returns LSM6DSO_NO_NEW_DATA (positive, not an error) when
 * the sensor has not produced a new accel+gyro sample since the last read.
 * ========================================================================== */

#define LSM6DSO_OK                0
#define LSM6DSO_NO_NEW_DATA       1
#define LSM6DSO_ERR_NOT_READY    -1   /* lsm6dso_update() before a good init */
#define LSM6DSO_ERR_BUS          -2   /* SPI transaction failed / timed out  */
#define LSM6DSO_ERR_WHO_AM_I     -3   /* register 0x0F did not read 0x6C     */

/*
 * The SPI bus must already be configured (mode 0 or 3, MSB first, <= 10 MHz)
 * by the platform layer before calling lsm6dso_init().
 *
 * Configuration: 1.666 kHz ODR, +/-16 g, +/-2000 dps, high-performance mode.
 */
int lsm6dso_init(hal_spi_bus_t bus);
int lsm6dso_update(void);

/* Last WHO_AM_I byte read during lsm6dso_init(), for diagnostics. */
uint8_t lsm6dso_last_who_am_i(void);

#endif
