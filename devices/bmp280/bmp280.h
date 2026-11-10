#ifndef LATTICE_BMP280_H
#define LATTICE_BMP280_H

#include <stdint.h>

/* BMP280 register definitions */
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

#define BMP280_STATUS_MEASURING     (1 << 3)

#define BMP280_SPI_READ             0x80
#define BMP280_SPI_WRITE_MASK       0x7F

/* Oversampling settings */
#define BMP280_OSRS_NONE            0x00
#define BMP280_OSRS_1X              0x01
#define BMP280_OSRS_2X              0x02
#define BMP280_OSRS_4X              0x03
#define BMP280_OSRS_8X              0x04
#define BMP280_OSRS_16X             0x05

/* Filter coefficients */
#define BMP280_FILTER_OFF           0x00
#define BMP280_FILTER_2             0x01
#define BMP280_FILTER_4             0x02
#define BMP280_FILTER_8             0x03
#define BMP280_FILTER_16            0x04

/* Power modes */
#define BMP280_MODE_SLEEP           0x00
#define BMP280_MODE_FORCED          0x01
#define BMP280_MODE_NORMAL          0x03

/* Standby times (ms) */
#define BMP280_STANDBY_0_5          0x00
#define BMP280_STANDBY_62_5         0x01
#define BMP280_STANDBY_125          0x02
#define BMP280_STANDBY_250          0x03
#define BMP280_STANDBY_500          0x03
#define BMP280_STANDBY_1000         0x05
#define BMP280_STANDBY_2000         0x06
#define BMP280_STANDBY_4000         0x07

/* Timing */
#define BMP280_RESET_DELAY_MS       10
#define BMP280_MEASURE_DELAY_MS     10
#define BMP280_POLL_TIMEOUT_MS      50
#define BMP280_POLL_PERIOD_MS       2

/* Default configuration */
#define BMP280_OSRS_T_DEFAULT       BMP280_OSRS_2X
#define BMP280_OSRS_P_DEFAULT       BMP280_OSRS_16X
#define BMP280_FILTER_DEFAULT       BMP280_FILTER_16
#define BMP280_MODE_DEFAULT         BMP280_MODE_FORCED

/* SPI Configuration */
#define BMP280_SPI_READ             0x80
#define BMP280_SPI_WRITE_MASK       0x7F

/* Calibration data structure */
typedef struct {
    uint16_t dig_t1;
    int16_t  dig_t2;
    int16_t  dig_t3;

    uint16_t dig_p1;
    int16_t  dig_p2;
    int16_t  dig_p3;
    int16_t  dig_p4;
    int16_t  dig_p5;
    int16_t  dig_p6;
    int16_t  dig_p7;
    int16_t  dig_p8;
    int16_t  dig_p9;

    int32_t  t_fine;
} bmp280_calib_t;

/* Sensor settings */
typedef struct {
    uint8_t osr_t;
    uint8_t osr_p;
    uint8_t filter;
    uint8_t mode;
    uint8_t standby;
} bmp280_settings_t;

/* Uncompensated data */
typedef struct {
    uint32_t pressure;
    uint32_t temperature;
} bmp280_uncomp_t;

/* Compensated data */
typedef struct {
    float temperature;
    float pressure;
} bmp280_data_t;

/* Device structure */
typedef struct {
    bmp280_calib_t calib;
    bmp280_settings_t settings;
    int32_t t_fine;
    int32_t last_temp_centi;
    uint32_t last_pressure_pa;
    bool ready;
} bmp280_dev_t;

/* SPI configuration (set by platform before calling bmp280_init) */
#define BMP280_SPI_BUS                HAL_SPI_BUS_2
#define BMP280_TIMEOUT_MS             100

/* Public API */
int bmp280_begin(bmp280_dev_t *dev, uint8_t osr_t, uint8_t osr_p, uint8_t filter, uint8_t mode);
int bmp280_read_calibration(bmp280_dev_t *dev);
int bmp280_read_raw(bmp280_dev_t *dev, bmp280_uncomp_t *uncomp);
int bmp280_compensate(bmp280_dev_t *dev, const bmp280_uncomp_t *uncomp, bmp280_data_t *data);
int bmp280_measure(bmp280_dev_t *dev, bmp280_data_t *data);
int bmp280_set_mode(bmp280_dev_t *dev, uint8_t mode);
int bmp280_set_settings(bmp280_dev_t *dev, uint8_t osr_t, uint8_t osr_p, uint8_t filter);
void bmp280_busy_wait(void);
bmp280_dev_t *bmp280_get_device(void);
#endif