/***********************************************************************************
 * FILE ID         : Main.c
 * VERSION NUMBER  : 1.0.0
 * PROJECT NAME    : BMP280 Barometer + LED Blink on THEJAS32
 * DESCRIPTION     : BMP280 SPI driver using HAL APIs with LED blink and UART output
 ************************************************************************************/

#include "board.h"
#include "hal_gpio.h"
#include "hal_timer.h"
#include "hal_uart.h"
#include "hal_spi.h"
#include "stdlib.h"
#include <math.h>
#include <string.h>

#define BMP280_CHIP_ID          0x58
#define BMP280_REG_ID           0xD0
#define BMP280_REG_RESET        0xE0
#define BMP280_RESET_VALUE      0xB6
#define BMP280_REG_STATUS       0xF3
#define BMP280_REG_CTRL_MEAS    0xF4
#define BMP280_REG_CONFIG       0xF5
#define BMP280_REG_CALIB        0x88
#define BMP280_REG_PRESS_MSB    0xF7
#define BMP280_REG_TEMP_MSB     0xFA

#define BMP280_STATUS_MEASURING (1 << 3)
#define BMP280_SPI_READ         0x80
#define BMP280_SPI_WRITE_MASK   0x7F

#define BMP280_RESET_DELAY_MS   10
#define BMP280_MEASURE_DELAY_MS 10
#define BMP280_POLL_TIMEOUT_MS  50
#define BMP280_POLL_PERIOD_MS   2

#define BMP280_OSRS_T           0x02
#define BMP280_OSRS_P           0x05
#define BMP280_IIR_FILTER       0x04

#define BARO_SPI_BUS            HAL_SPI_BUS_2
#define BARO_CS_MODE            HAL_SPI_CS_MODE_CSAAT
#define BARO_HW_CS              HAL_SPI_CS_HW_0
#define BARO_BAUD_DIV           HAL_SPI_BAUD_DIV_32
#define BARO_TIMEOUT_MS         100

static void delay_ms(uint32_t ms)
{
    hal_timer_delay_ms(ms);
}

static int bmp280_read_regs(uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t cmd = reg | BMP280_SPI_READ;
    return hal_spi_write_read(BARO_SPI_BUS, &cmd, 1, buf, len, BARO_TIMEOUT_MS);
}

static int bmp280_write_reg(uint8_t reg, uint8_t value)
{
    uint8_t tx[2] = { reg & BMP280_SPI_WRITE_MASK, value };
    return hal_spi_write(BARO_SPI_BUS, tx, 2, BARO_TIMEOUT_MS);
}

static uint16_t le16u(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static int16_t le16s(const uint8_t *p)
{
    return (int16_t)le16u(p);
}

typedef struct {
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

typedef struct {
    int32_t t_fine;
    int32_t last_temp_centi;
    uint32_t last_pressure_pa;
    bmp280_calib_t calib;
} bmp280_dev_t;

static int bmp280_calib_valid(const bmp280_calib_t *c)
{
    if ((c->T1 == 0) || (c->P1 == 0)) return 0;
    if ((c->T1 == 0xFFFF) || (c->P1 == 0xFFFF)) return 0;
    if ((c->T2 == 0) && (c->T3 == 0)) return 0;
    return 1;
}

static int bmp280_init(bmp280_dev_t *dev)
{
    uint8_t id;
    uint8_t calib[24];
    uint8_t i;

    hal_spi_set_cs(BARO_SPI_BUS, false);
    delay_ms(BMP280_RESET_DELAY_MS);

    if (bmp280_read_regs(BMP280_REG_ID, &id, 1) != 0) return -1;
    if (id != BMP280_CHIP_ID) return -2;

    if (bmp280_write_reg(BMP280_REG_RESET, BMP280_RESET_VALUE) != 0) return -3;
    delay_ms(BMP280_RESET_DELAY_MS);

    for (i = 0; i < 24; i++) {
        if (bmp280_read_regs((uint8_t)(BMP280_REG_CALIB + i), &calib[i], 1) != 0) return -4;
    }

    dev->calib.T1 = le16u(&calib[0]);
    dev->calib.T2 = le16s(&calib[2]);
    dev->calib.T3 = le16s(&calib[4]);
    dev->calib.P1 = le16u(&calib[6]);
    dev->calib.P2 = le16s(&calib[8]);
    dev->calib.P3 = le16s(&calib[10]);
    dev->calib.P4 = le16s(&calib[12]);
    dev->calib.P5 = le16s(&calib[14]);
    dev->calib.P6 = le16s(&calib[16]);
    dev->calib.P7 = le16s(&calib[18]);
    dev->calib.P8 = le16s(&calib[20]);
    dev->calib.P9 = le16s(&calib[22]);

    if (!bmp280_calib_valid(&dev->calib)) {
        delay_ms(BMP280_RESET_DELAY_MS);
        for (i = 0; i < 24; i++) {
            bmp280_read_regs((uint8_t)(BMP280_REG_CALIB + i), &calib[i], 1);
        }
        dev->calib.T1 = le16u(&calib[0]);
        dev->calib.T2 = le16s(&calib[2]);
        dev->calib.T3 = le16s(&calib[4]);
        dev->calib.P1 = le16u(&calib[6]);
        dev->calib.P2 = le16s(&calib[8]);
        dev->calib.P3 = le16s(&calib[10]);
        dev->calib.P4 = le16s(&calib[12]);
        dev->calib.P5 = le16s(&calib[14]);
        dev->calib.P6 = le16s(&calib[16]);
        dev->calib.P7 = le16s(&calib[18]);
        dev->calib.P8 = le16s(&calib[20]);
        dev->calib.P9 = le16s(&calib[22]);
        if (!bmp280_calib_valid(&dev->calib)) return -5;
    }

    bmp280_write_reg(BMP280_REG_CONFIG, (uint8_t)(BMP280_IIR_FILTER << 2));
    bmp280_write_reg(BMP280_REG_CTRL_MEAS,
                     (uint8_t)((BMP280_OSRS_T << 5) | (BMP280_OSRS_P << 2) | 0x00));

    return 0;
}

static int bmp280_read(bmp280_dev_t *dev, float *temperature_c, float *pressure_pa)
{
    uint8_t pbuf[3], tbuf[3], status;
    uint8_t i;
    uint32_t timeout;
    int32_t adc_P, adc_T;
    int64_t v1_64, v2_64, tf_64, temp_64;
    int32_t var1, var2, temp;
    uint32_t p;

    bmp280_write_reg(BMP280_REG_CTRL_MEAS,
                     (uint8_t)((BMP280_OSRS_T << 5) | (BMP280_OSRS_P << 2) | 0x01));
    delay_ms(BMP280_MEASURE_DELAY_MS);

    timeout = BMP280_POLL_TIMEOUT_MS;
    for (;;) {
        bmp280_read_regs(BMP280_REG_STATUS, &status, 1);
        if ((status & BMP280_STATUS_MEASURING) == 0) break;
        if (timeout < BMP280_POLL_PERIOD_MS) return -6;
        delay_ms(BMP280_POLL_PERIOD_MS);
        timeout -= BMP280_POLL_PERIOD_MS;
    }

    for (i = 0; i < 3; i++) {
        bmp280_read_regs((uint8_t)(BMP280_REG_PRESS_MSB + i), &pbuf[i], 1);
        bmp280_read_regs((uint8_t)(BMP280_REG_TEMP_MSB + i), &tbuf[i], 1);
    }

    adc_P = ((int32_t)pbuf[0] << 12) | ((int32_t)pbuf[1] << 4) | ((int32_t)pbuf[2] >> 4);
    adc_T = ((int32_t)tbuf[0] << 12) | ((int32_t)tbuf[1] << 4) | ((int32_t)tbuf[2] >> 4);

    if (((adc_P == 0) || (adc_P == 0xFFFFF)) && ((adc_T == 0) || (adc_T == 0xFFFFF))) return -7;

    v1_64 = (((int64_t)(adc_T >> 3) - ((int64_t)dev->calib.T1 << 1)) * (int64_t)dev->calib.T2) >> 11;
    v2_64 = (((((int64_t)(adc_T >> 4) - (int64_t)dev->calib.T1) *
              ((int64_t)(adc_T >> 4) - (int64_t)dev->calib.T1)) >> 12) *
             (int64_t)dev->calib.T3) >> 14;
    tf_64 = v1_64 + v2_64;
    if (tf_64 > INT32_MAX) tf_64 = INT32_MAX;
    else if (tf_64 < INT32_MIN) tf_64 = INT32_MIN;
    dev->t_fine = (int32_t)tf_64;

    temp_64 = (tf_64 * 5 + 128) >> 8;
    if (temp_64 > INT32_MAX) temp_64 = INT32_MAX;
    else if (temp_64 < INT32_MIN) temp_64 = INT32_MIN;
    temp = (int32_t)temp_64;
    dev->last_temp_centi = temp;

    if (temperature_c) *temperature_c = (float)temp / 100.0f;

    var1 = (((int32_t)dev->t_fine) / 2) - 64000;
    var2 = (((var1 / 4) * (var1 / 4)) / 2048) * ((int32_t)dev->calib.P6);
    var2 = var2 + ((var1 * ((int32_t)dev->calib.P5)) * 2);
    var2 = (var2 / 4) + (((int32_t)dev->calib.P4) * 65536);
    var1 = ((((int32_t)dev->calib.P3 * (((var1 / 4) * (var1 / 4)) / 8192)) / 8) +
            ((((int32_t)dev->calib.P2) * var1) / 2)) / 262144;
    var1 = ((((32768 + var1)) * ((int32_t)dev->calib.P1)) / 32768);

    if (var1 == 0) {
        p = 0;
    } else {
        p = (uint32_t)(((int32_t)(1048576 - adc_P) - (var2 / 4096)) * 3125);
        if (p < 0x80000000U) p = (p << 1) / (uint32_t)var1;
        else p = (p / (uint32_t)var1) * 2;
        var1 = (((int32_t)dev->calib.P9) * ((int32_t)(((p / 8) * (p / 8)) / 8192))) / 4096;
        var2 = (((int32_t)(p / 4)) * ((int32_t)dev->calib.P8)) / 8192;
        p = (uint32_t)((int32_t)p + ((var1 + var2 + dev->calib.P7) / 16));
    }

    if (pressure_pa) *pressure_pa = (float)p;
    dev->last_pressure_pa = p;

    return 0;
}

static float bmp280_pressure_to_altitude(float pressure_pa)
{
    if (pressure_pa <= 0.0f) return 0.0f;
    return 44330.0f * (1.0f - powf((pressure_pa / 101325.0f), 0.190284f));
}

static void uart_print(const char *s)
{
    hal_uart_write(HAL_UART_0, (const uint8_t *)s, strlen(s));
}

static void uart_print_int(int32_t v)
{
    char buf[16];
    if (v < 0) {
        uart_print("-");
        v = -v;
    }
    int i = 0;
    do { buf[i++] = '0' + (v % 10); v /= 10; } while (v > 0);
    while (i--) hal_uart_write(HAL_UART_0, (const uint8_t *)&buf[i], 1);
}

static void uart_print_centi(int32_t centi)
{
    uint32_t mag = (centi < 0) ? (uint32_t)(-centi) : (uint32_t)centi;
    if (centi < 0) uart_print("-");
    uart_print_int(mag / 100);
    uart_print(".");
    uart_print_int((mag % 100) / 10);
    uart_print_int(mag % 10);
}

static void uart_print_u32(uint32_t v)
{
    char buf[12];
    int i = 0;
    do { buf[i++] = '0' + (v % 10); v /= 10; } while (v > 0);
    while (i--) hal_uart_write(HAL_UART_0, (const uint8_t *)&buf[i], 1);
}

static void uart_print_float(float f)
{
    int32_t ipart = (int32_t)f;
    float fpart = f - (float)ipart;
    if (fpart < 0) fpart = -fpart;
    uart_print_int(ipart);
    uart_print(".");
    uart_print_int((int32_t)(fpart * 100.0f + 0.5f));
}

int main(void)
{
    hal_gpio_init();
    board_init();

    hal_uart_config_t ucfg = { .baud_rate = 115200, .data_bits = 8, .stop_bits = 1, .parity = false };
    hal_uart_init(HAL_UART_0, &ucfg);

    hal_spi_config_t scfg = {
        .mode = HAL_SPI_MODE_3,
        .freq_hz = 0,
        .baud_div = BARO_BAUD_DIV,
        .bit_order = HAL_SPI_BIT_ORDER_MSB,
        .cs_mode = BARO_CS_MODE,
        .hw_cs = BARO_HW_CS,
        .word_size = 8
    };
    hal_spi_init(BARO_SPI_BUS, &scfg);

    hal_gpio_write(BOARD_LED_RED_PIN, HAL_GPIO_LEVEL_LOW);

    uart_print("\r\nBMP280 barometer (THEJAS32, SPI2, SS0)\r\n");

    bmp280_dev_t dev = {0};
    int ret = bmp280_init(&dev);
    if (ret != 0) {
        uart_print("ERROR: BMP280 init failed (code ");
        uart_print_int(ret);
        uart_print(")\r\n");
        for (;;) delay_ms(1000);
    }

    uart_print("Ready: forced mode, temp x2, pressure x16, IIR x16\r\n");

    uint32_t sample_count = 0;
    float temperature_c, pressure_pa, altitude_m;

    while (1) {
        ret = bmp280_read(&dev, &temperature_c, &pressure_pa);
        if (ret == 0) {
            altitude_m = bmp280_pressure_to_altitude(pressure_pa);
            if (++sample_count >= 5) {
                sample_count = 0;
                uart_print("temp=");
                uart_print_centi(dev.last_temp_centi);
                uart_print(" C  pressure=");
                uart_print_u32(dev.last_pressure_pa);
                uart_print(" Pa  altitude=");
                uart_print_float(altitude_m);
                uart_print(" m\r\n");
            }
        }

        hal_gpio_write(BOARD_LED_RED_PIN, HAL_GPIO_LEVEL_HIGH);
        uart_print("LED ON\r\n");
        delay_ms(500);

        hal_gpio_write(BOARD_LED_RED_PIN, HAL_GPIO_LEVEL_LOW);
        uart_print("LED OFF\r\n");
        delay_ms(500);
    }

    return 0;
}

void vApplicationMallocFailedHook(void)
{
    for (;;);
}

void vApplicationIdleHook(void) { }

void vApplicationStackOverflowHook(void *pxTask, char *pcTaskName)
{
    (void)pxTask; (void)pcTaskName;
    for (;;);
}

void vApplicationTickHook(void) { }