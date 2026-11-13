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
#include "timer.h"          /* TIMER TEST: raw TIMER_2 register */
#include <stdio.h>
#include <stdint.h>
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
 * MATH TEST: is 64-bit division correct on this build?
 * The operands are volatile so the compiler can't work the answer out
 * at build time - the board itself has to do the division.
 * ========================================================================== */
static void math_test_line(const char *name, uint32_t got, uint32_t want)
{
    uart_print(got == want ? "  PASS  " : "  FAIL  ");
    uart_print(name);
    uart_print("  got=");
    uart_print_u32(got);
    uart_print("  want=");
    uart_print_u32(want);
    uart_print("\r\n");
}

static void math_test(void)
{
    volatile uint32_t a32 = 4310000U, b32 = 1000U;
    volatile uint64_t a64 = 4310000ULL, b64 = 1000ULL;
    volatile uint64_t big = 10000000000ULL;            /* 1e10, needs 64 bits */
    volatile int64_t  s_num = -(10LL << 16);           /* -10.0 in Q16.16 */
    volatile int32_t  fa = 10 << 16, fb = 4 << 16;     /* 10.0 and 4.0    */

    uart_print("\r\nMATH TEST\r\n");
    math_test_line("u32 4310000/1000      ", a32 / b32, 4310U);
    math_test_line("u64 4310000/1000      ", (uint32_t)(a64 / b64), 4310U);
    math_test_line("u64 1e10/1000         ", (uint32_t)(big / b64), 10000000U);
    math_test_line("u64 1e10%1000         ", (uint32_t)(big % b64), 0U);
    math_test_line("s64 -10.0/4 (Q16,neg) ", (uint32_t)(-(int32_t)(s_num / 4)), 163840U);
    /* exactly what fix16_div does: ((int64)a << 16) / b  -> 10/4 = 2.5 */
    math_test_line("fix16_div 10.0/4.0    ",
                   (uint32_t)((((int64_t)fa) << 16) / (int64_t)fb), 163840U);
    /* soft-float (no FPU): also done by libgcc helpers */
    {
        volatile float fa = 10.0f, fb = 4.0f;
        math_test_line("float 10*4            ", (uint32_t)(fa * fb), 40U);
        math_test_line("float 10/4 x1000      ", (uint32_t)(fa / fb * 1000.0f), 2500U);
        math_test_line("float 10-4.5 x10      ", (uint32_t)((fa - 4.5f) * 10.0f), 55U);
    }
    uart_print("END MATH TEST\r\n\r\n");
}

/* ============================================================================
 * MAIN
 * ========================================================================== */

int main(void)
{
    bool led_on = false;

    app_init();
    math_test();                                  /* MATH TEST */

    uint32_t prev_us = hal_time_us();             /* TIMER TEST */

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

        /* TIMER TEST (diagnostic): raw counter + time worked out two ways */
        uint32_t raw    = Timer(TIMER_2).CurrentValue;
        uint32_t now_us = hal_time_us();
        uint32_t ms_a   = now_us / 1000U;              /* 32-bit divide        */
        uint32_t ms_b   = (uint32_t)hal_time_ms();     /* 64-bit divide        */
        uart_print("loop_us=");
        uart_print_u32(now_us - prev_us);
        uart_print("  us/1000=");
        uart_print_u32(ms_a);
        uart_print("  time_ms=");
        uart_print_u32(ms_b);
        uart_print("  raw=");
        uart_print_u32(raw);
        uart_print("\r\n");
        prev_us = now_us;
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
