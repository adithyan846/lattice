/***********************************************************************************
 * FILE ID         : main.c
 * VERSION         : 1.1.0
 * PROJECT         : LATTICE - ARIES V3
 * DESCRIPTION     : Barometer + IMU application (BMP280, LSM6DSO) via sensor interfaces
 ***********************************************************************************/

#include "hal_gpio.h"
#include "hal_timer.h"
#include "hal_uart.h"
#include "barometer.h"
#include "bmp280.h"
#include "imu.h"
#include "lsm6dso.h"
#include "board.h"
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * APPLICATION CONFIGURATION
 * ========================================================================== */

#define LED_PIN                 BOARD_LED_RED_PIN
#define LOOP_PERIOD_MS          500
#define IMU_RETRY_LOOPS         6       /* retry a failed IMU init every ~3 s */

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

static void uart_print_hex8(uint8_t v)
{
    static const char hex[] = "0123456789ABCDEF";
    char text[2] = { hex[(v >> 4) & 0x0F], hex[v & 0x0F] };
    hal_uart_write(HAL_UART_0, (const uint8_t *)text, 2);
}

/* Print three milli-unit values with two decimals (e.g. 1234 -> 1.23). */
static void uart_print_milli3(const char *label, const int32_t v[3])
{
    static const char axis[3] = { 'X', 'Y', 'Z' };

    uart_print(label);
    for (int i = 0; i < 3; i++) {
        char name[4] = { ' ', axis[i], '=', '\0' };
        uart_print(name);
        uart_print_centi(v[i] / 10);
    }
}

/* ============================================================================
 * APPLICATION INITIALISATION
 * ========================================================================== */

static bool baro_ok = false;
static bool imu_ok = false;

static void imu_try_init(void)
{
    int ret = imu_init();
    if (ret != 0) {
        uart_print("ERROR: IMU init failed (code ");
        uart_print_int(ret);
        uart_print(", WHO_AM_I 0x");
        uart_print_hex8(lsm6dso_last_who_am_i());
        uart_print(", expected 0x6C)\r\n");
    } else {
        imu_ok = true;
        uart_print("IMU ready: 1666 Hz, +/-16 g, +/-2000 dps\r\n");
    }
}

static void app_init(void)
{
    int ret;

    /* GPIO and sensor SPI buses (SPI1 = IMU, SPI2 = barometer) */
    ret = board_init();
    if (ret != 0) {
        uart_print("ERROR: board init failed (code ");
        uart_print_int(ret);
        uart_print(")\r\n");
        return;
    }

    /* UART_0 is already running at 115200 from the bootloader */
    hal_uart_config_t ucfg = {
        .baud_rate = 115200,
        .data_bits = 8,
        .stop_bits = 1,
        .parity = false
    };
    hal_uart_init(HAL_UART_0, &ucfg);

    uart_print("\r\nLattice: BMP280 (SPI2/J4) + LSM6DSO (SPI1/J7)\r\n");

    ret = barometer_init();
    if (ret != 0) {
        uart_print("ERROR: barometer init failed (code ");
        uart_print_int(ret);
        uart_print(", chip ID 0x");
        uart_print_hex8(bmp280_last_chip_id());
        uart_print(", expected 0x58)\r\n");
    } else {
        baro_ok = true;
        uart_print("Barometer ready: forced mode, temp x2, pressure x16, IIR x16\r\n");
    }

    imu_try_init();
}

/* ============================================================================
 * APPLICATION UPDATE
 * ========================================================================== */

static void app_update(void)
{
    int ret;

    if (baro_ok) {
        barometer_data_t baro;

        ret = barometer_update();
        if (ret != 0) {
            uart_print("ERROR: barometer read failed (code ");
            uart_print_int(ret);
            uart_print(")\r\n");
        } else if (barometer_get_latest(&baro)) {
            uart_print("temp=");
            uart_print_centi(baro.temperature_centi_c);
            uart_print(" C  pressure=");
            uart_print_u32(baro.pressure_pa);
            uart_print(" Pa  altitude=");
            uart_print_int(baro.altitude_cm);
            uart_print(" cm\r\n");
        }
    }

    if (!imu_ok) {
        static uint32_t retry_count = 0;

        if (++retry_count >= IMU_RETRY_LOOPS) {
            retry_count = 0;
            imu_try_init();
        }
    }

    if (imu_ok) {
        imu_data_t imu;

        ret = imu_update();
        if (ret < 0) {
            uart_print("ERROR: IMU read failed (code ");
            uart_print_int(ret);
            uart_print(")\r\n");
        } else if (imu_get_latest(&imu)) {
            uart_print_milli3("gyro[dps]", imu.gyro_mdps);
            uart_print_milli3(" | accel[g]", imu.accel_mg);
            uart_print("\r\n");
        }
    }
}

/* ============================================================================
 * MAIN
 * ========================================================================== */

int main(void)
{
    bool led_on = false;

    app_init();

    while (1) {
        app_update();

        /*
         * Heartbeat LED.  State is tracked here rather than with
         * hal_gpio_toggle(): the BSP's GPIO_read_pin() switches the pin to
         * input, which would leave the LED floating.
         */
        led_on = !led_on;
        hal_gpio_write(LED_PIN, led_on ? HAL_GPIO_LEVEL_HIGH : HAL_GPIO_LEVEL_LOW);
        hal_timer_delay_ms(LOOP_PERIOD_MS);
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
