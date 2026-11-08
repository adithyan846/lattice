#ifndef LATTICE_BMP280_H
#define LATTICE_BMP280_H

#include "hal_spi.h"

/* ============================================================================
 * ERROR CODES
 *
 * All BMP280 functions return 0 on success or one of these negative codes.
 * HAL SPI errors are reported as BMP280_ERR_BUS.
 * ========================================================================== */

#define BMP280_OK                0
#define BMP280_ERR_NOT_READY    -1   /* bmp280_update() before a good init   */
#define BMP280_ERR_BUS          -2   /* SPI transaction failed / timed out   */
#define BMP280_ERR_CHIP_ID      -3   /* register 0xD0 did not read 0x58      */
#define BMP280_ERR_CALIB        -4   /* calibration block all 0x00 / 0xFF    */
#define BMP280_ERR_TIMEOUT      -5   /* measurement never finished           */
#define BMP280_ERR_DATA         -6   /* ADC values look like a dead bus      */
#define BMP280_ERR_COMPENSATE   -7   /* compensation divide-by-zero          */

/*
 * The SPI bus must already be configured (mode 0 or 3, MSB first, <= 10 MHz)
 * by the platform layer before calling bmp280_init().
 */
int bmp280_init(hal_spi_bus_t bus);
int bmp280_update(void);

/* Last chip ID byte read during bmp280_init(), for diagnostics. */
uint8_t bmp280_last_chip_id(void);

#endif
