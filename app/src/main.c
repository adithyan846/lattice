/***********************************************************************************
 * FILE ID         : main.c
 * VERSION         : 1.0.0
 * PROJECT         : LATTICE - ARIES V3
 * DESCRIPTION     : Barometer application using BMP280 via barometer interface
 ***********************************************************************************/

#include "hal_gpio.h"
#include "hal_timer.h"
#include "hal_uart.h"
#include "hal_spi.h"
#include "barometer.h"
#include "board.h"
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * BAROMETER SPI CONFIGURATION
 * ========================================================================== */

#define BARO_SPI_BUS            HAL_SPI_BUS_2
#define BARO_CS_MODE            HAL_SPI_CS_MODE_CSAAT
#define BARO_HW_CS              HAL_SPI_CS_HW_0
#define BARO_BAUD_DIV           HAL_SPI_BAUD_DIV_32
#define BARO_TIMEOUT_MS         100

#define LED_PIN                 BOARD_LED_RED_PIN
#define UART_BAUD               115200

/* ============================================================================
 * UART PRINT HELPERS
 * ========================================================================== */

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

/* ============================================================================
 * APPLICATION INITIALISATION
 * ========================================================================== */

static void app_init(void)
{
    /* Initialize hardware */
    hal_gpio_init();
    board_init();

    /* Initialize UART for console output */
    hal_uart_config_t ucfg = {
        .baud_rate = UART_BAUD,
        .data_bits = 8,
        .stop_bits = 1,
        .parity = false
    };
    hal_uart_init(HAL_UART_0, &ucfg);

    /* Initialize SPI for barometer */
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

    /* Initialize barometer interface */
    int ret = barometer_init();
    if (ret != 0) {
        uart_print("ERROR: Barometer init failed (code ");
        uart_print_int(ret);
        uart_print(")\r\n");
        return;
    }

    uart_print("Barometer ready\r\n");
}

/* ============================================================================
 * APPLICATION UPDATE
 * ========================================================================== */

static void app_update(void)
{
    barometer_data_t data;

    /* Trigger measurement and get latest data */
    int ret = barometer_update();
    if (ret != 0) {
        return;
    }

    /* Get latest measurement */
    if (barometer_get_latest(&data)) {
        uart_print("temp=");
        uart_print_centi((int32_t)(data.temperature_c * 100));
        uart_print(" C  pressure=");
        uart_print_u32((uint32_t)data.pressure_pa);
        uart_print(" Pa\r\n");
    }
}

/* ============================================================================
 * MAIN
 * ========================================================================== */

int main(void)
{
    app_init();

    while (1) {
        app_update();

        /* Blink LED */
        hal_gpio_write(LED_PIN, HAL_GPIO_LEVEL_HIGH);
        uart_print("LED ON\r\n");
        hal_timer_delay_ms(500);

        hal_gpio_write(LED_PIN, HAL_GPIO_LEVEL_LOW);
        uart_print("LED OFF\r\n");
        hal_timer_delay_ms(500);
    }

    return 0;
}

/* ============================================================================
 * FREERTOS HOOKS
 * ========================================================================== */

void vApplicationMallocFailedHook(void) { for (;;); }
void vApplicationIdleHook(void) { }
void vApplicationStackOverflowHook(void *pxTask, char *pcTaskName) { (void)pxTask; (void)pcTaskName; for (;;); }
void vApplicationTickHook(void) { }
