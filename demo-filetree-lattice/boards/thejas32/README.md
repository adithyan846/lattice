# THEJAS32 board support package

Reference BSP for the THEJAS32 RISC-V target. The files here are intentionally
thin: they translate the HAL interfaces into SoC registers and leave policy to
the flight modules.

| File | Responsibility |
|---|---|
| `board.c` | Boot, clock, bus init, fault hooks, FreeRTOS static hooks |
| `board.h` | Board identity and init API |
| `pinmap.h` | Pin and peripheral mapping (placeholders) |
| `hal/uart_ll.c` | UART low-level driver |
| `hal/spi_ll.c` | SPI low-level driver |
| `hal/i2c_ll.c` | I2C low-level driver |
| `hal/gpio_ll.c` | GPIO and interrupts |
| `hal/pwm_ll.c` | Motor PWM/DShot outputs |
| `hal/adc_ll.c` | Battery/board ADC |
| `hal/flash_ll.c` | Parameter/mission flash partitions |
| `hal/sd_ll.c` | SD card block interface |
| `ld/link.ld` | Memory map |
| `startup.S` | Reset vector and C runtime entry |

## Bring-up order

1. Clock and console UART.
2. GPIO LED and safety switch.
3. IMU SPI.
4. PWM outputs (props removed).
5. RC UART.
6. Telemetry UART.
7. GNSS UART.
8. Battery ADC.
9. SD card.
10. Flash partitions.

## Memory map

`ld/link.ld` uses placeholder origins. Replace them with the actual flash/RAM
addresses from the THEJAS32 manual and add `ASSERT` limits for the main stack,
FreeRTOS heap and `.bss`. A wrong memory map can silently overlap the heap with
the stack.
