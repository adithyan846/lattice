# API Reference

---
tags: [api, reference, bsp, freertos]
status: active
created: 2026-11-01
---

# API Reference

> All available headers and functions for development on THEJAS32.

## Vendor BSP Headers (from freertos-thejas32)

Located in `freertos-thejas32/Demo/RISCV_CDAC_THEJAS32/thejas32_hal/drivers/vega-sdk_freertos_thejas32/bsp/include/`

| Header | Purpose | Key Functions |
|--------|---------|---------------|
| `debug_uart.h` | Debug UART @ 115200 | `init_uart()`, `Transmit_uart()`, `tx_uart()`, `rx_uart()`, `get_hex()`, `get_decimal()` |
| `uart.h` | UART driver | `uart_init()`, `uart_putchar()`, `uart_getchar()`, `uart_configure()` |
| `gpio.h` | GPIO control | `gpio_init()`, `gpio_set()`, `gpio_clear()`, `gpio_toggle()`, `gpio_read()` |
| `led.h` | LED control | `led_init()`, `vToggleLED()`, `led_on()`, `led_off()` |
| `i2c.h` | I2C bus | `i2c_init()`, `i2c_write()`, `i2c_read()`, `i2c_write_read()` |
| `spi.h` | SPI bus | `spi_init()`, `spi_transfer()`, `spi_write()`, `spi_read()` |
| `timer.h` | Timer | `timer_init()`, `timer_start()`, `timer_stop()`, `timer_get_count()` |
| `interrupt.h` | Interrupts | `interrupt_enable()`, `interrupt_disable()`, `interrupt_configure()` |
| `pwmc.h` | PWM | `pwmc_init()`, `pwmc_set_duty()`, `pwmc_set_period()`, `pwmc_start()`, `pwmc_stop()` |
| `adc.h` | ADC | `adc_init()`, `adc_read()`, `adc_config_channel()` |
| `qspi.h` | QSPI flash | `qspi_init()`, `qspi_read()`, `qspi_write()`, `qspi_erase()` |
| `at25sf161.h` | Flash storage | `at25sf161_init()`, `at25sf161_read()`, `at25sf161_write()`, `at25sf161_erase()` |
| `encoding.h` | CSR encoding | `csr_read()`, `csr_write()` |
| `config.h` | Board config | `UART_BASE`, `configMTIMECMP_BASE_ADDRESS`, etc. |

## FreeRTOS Kernel APIs

| Header | Purpose |
|--------|---------|
| `FreeRTOS.h` | Core definitions |
| `task.h` | Task management (`xTaskCreate`, `vTaskStartScheduler`, `vTaskDelay`) |
| `queue.h` | Message queues (`xQueueCreate`, `xQueueSend`, `xQueueReceive`) |
| `semphr.h` | Semaphores/mutexes (`xSemaphoreCreateBinary`, `xSemaphoreTake`) |
| `timers.h` | Software timers (`xTimerCreate`, `xTimerStart`) |
| `event_groups.h` | Event groups (`xEventGroupCreate`, `xEventGroupSetBits`) |
| `stream_buffer.h` | Stream buffers (`xStreamBufferCreate`, `xStreamBufferSend`) |
| `croutine.h` | Co-routines |

## Vendor Types

Defined in `uart.h`:
```c
typedef unsigned char UC;    // 1 Byte
typedef unsigned short US;   // 2 Bytes
typedef unsigned int UI;     // 4 Bytes
typedef unsigned long UL;    // 8 Bytes
```

## Example Usage

```c
#include <FreeRTOS.h>
#include <task.h>
#include "debug_uart.h"
#include "i2c.h"
#include "gpio.h"

int main(void) {
    init_uart();                    // 115200 baud
    gpio_init();
    i2c_init(I2C_0, 400000);
    
    Transmit_uart((UC*)"Hello FC\n\r");
    
    xTaskCreate(flight_task, "Flight", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY+1, NULL);
    vTaskStartScheduler();
}
```

## Your Interfaces (To Define)

| File | Purpose |
|------|---------|
| `interfaces/sensor/imu.h` | `imu_driver_t` — `init()`, `read()`, `self_test()` |
| `interfaces/sensor/barometer.h` | `baro_driver_t` — `init()`, `read()` |
| `interfaces/sensor/gnss.h` | `gnss_driver_t` — `init()`, `read()` |
| `interfaces/actuator/esc.h` | `esc_driver_t` — `init()`, `write()`, `arm()`, `disarm()` |

## Navigation

- [[Developer Notes]] — Project overview and phases
- [[Architecture Details]] — Detailed layer specifications
- [[Makefile Evolution]] — How the Makefile grows across phases
- [[Flash Workflow]] — Flashing to SRAM vs SPI FLASH

All notes link back to [[_index.md]].
