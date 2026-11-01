#include "board.h"

#include <stdio.h>
#include <string.h>

#include <freertrtos/fault.h>
#include <hal/hal.h>

#include "FreeRTOS.h"
#include "task.h"

#include "pinmap.h"

static StaticTask_t idle_task_buffer;
static StackType_t idle_task_stack[configMINIMAL_STACK_SIZE];

static StaticTask_t timer_task_buffer;
static StackType_t timer_task_stack[configTIMER_TASK_STACK_DEPTH];

int board_clock_init(void)
{
    /* TODO: configure PLL, bus clocks and peripheral clocks from the board
     * schematic. Keep the CPU clock consistent with configCPU_CLOCK_HZ. */
    return 0;
}

int board_init(void)
{
    if (board_clock_init() != 0) {
        return -1;
    }

    /* Safe state first: motors must not twitch during bus init. */
    hal_pwm_init();
    hal_pwm_disarm_all();

    if (hal_gpio_init(HAL_GPIO_LED_STATUS, HAL_GPIO_MODE_OUTPUT) != 0) {
        return -1;
    }

    if (hal_gpio_init(HAL_GPIO_SAFETY_SWITCH, HAL_GPIO_MODE_INPUT_PULLUP) != 0) {
        return -1;
    }

    if (hal_uart_init(HAL_UART_CONSOLE, &(hal_uart_config_t){
            .baudrate = 115200u, .data_bits = 8u, .stop_bits = 1u,
            .parity = HAL_UART_PARITY_NONE}) != 0) {
        return -1;
    }

    if (hal_uart_init(HAL_UART_GNSS, &(hal_uart_config_t){
            .baudrate = 115200u, .data_bits = 8u, .stop_bits = 1u,
            .parity = HAL_UART_PARITY_NONE}) != 0) {
        return -1;
    }

    if (hal_uart_init(HAL_UART_TELEMETRY, &(hal_uart_config_t){
            .baudrate = 57600u, .data_bits = 8u, .stop_bits = 1u,
            .parity = HAL_UART_PARITY_NONE}) != 0) {
        return -1;
    }

    if (hal_spi_init(HAL_SPI_IMU, 8000000u, 0u) != 0) {
        return -1;
    }

    if (hal_i2c_init(HAL_I2C_MAG, 400000u) != 0) {
        return -1;
    }

    if (hal_adc_init() != 0) {
        return -1;
    }

    if (hal_flash_init() != 0) {
        return -1;
    }

    (void)hal_sd_init();

    return 0;
}

void board_reset(void)
{
    hal_system_reset();
}

uint32_t board_unique_id(void)
{
    /* TODO: read the SoC unique ID register. */
    return 0x00000001u;
}

const char *board_name(void)
{
    return "thejas32";
}

void frt_assert_failed(const char *file, int line)
{
    char message[96];
    const int length = snprintf(message, sizeof(message), "\r\nASSERT %s:%d\r\n", file, line);

    if (length > 0) {
        (void)hal_uart_write(HAL_UART_CONSOLE, (const uint8_t *)message, (size_t)length, 10u);
    }

    frt_fault_reboot();
}

void frt_stack_overflow_hook(void *task_handle, const char *task_name)
{
    (void)task_handle;
    (void)task_name;
    frt_fault_reboot();
}

void frt_malloc_failed_hook(void)
{
    frt_fault_reboot();
}

void frt_fault_reboot(void)
{
    hal_pwm_disarm_all();
    hal_system_reset();
}

/* FreeRTOS static-allocation hooks. */
void vApplicationGetIdleTaskMemory(StaticTask_t **idle_task_tcb,
                                   StackType_t **idle_task_stack_buffer,
                                   uint32_t *idle_task_stack_size)
{
    *idle_task_tcb = &idle_task_buffer;
    *idle_task_stack_buffer = idle_task_stack;
    *idle_task_stack_size = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **timer_task_tcb,
                                    StackType_t **timer_task_stack_buffer,
                                    uint32_t *timer_task_stack_size)
{
    *timer_task_tcb = &timer_task_buffer;
    *timer_task_stack_buffer = timer_task_stack;
    *timer_task_stack_size = configTIMER_TASK_STACK_DEPTH;
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *task_name)
{
    frt_stack_overflow_hook(task, task_name);
}

void vApplicationMallocFailedHook(void)
{
    frt_malloc_failed_hook();
}
