# LED Blinking — GPIO + Polled Timer

---
tags: [led, gpio, timer, bring-up, debug]
status: active
created: 2026-11-02
updated: 2026-11-02
---

# LED Blinking — GPIO + Polled Timer

> Continuous LED blinking on `app/src/main.c` for the CDAC VEGA THEJAS32 (ARIES) RISC-V board. Uses GPIO + polled timer delay — **no FreeRTOS scheduler tick required** — to avoid the machine-timer-interrupt reset that occurred when `vTaskStartScheduler()` was called.

## Problem

The original `app/src/main.c` called `vTaskStartScheduler()`, which set up the machine timer (mtimer) compare register and enabled machine-timer interrupts. On the first tick interrupt, the board reset automatically right after printing `hiii 5` — before any task could run.

Root cause: `main.c` overrode the weak `vPortSetupTimerInterrupt()` from `port.c` with a 32-bit-pointer version of `pullMachineTimerCompareRegister`, while `port.c` defines it as `volatile uint64_t *` and `portASM.S` expects a 64-bit pointer. The type mismatch plus the timer ISR corrupting memory caused the reset.

## Solution

Removed the `vPortSetupTimerInterrupt()` override and the mismatched `extern` declarations from `app/src/main.c`, letting `port.c`'s weak version handle timer setup. Then replaced the FreeRTOS-task-based blink with a **polled blink loop** in `main()` using `timer_put_delay()` (no interrupts, no scheduler tick required).

### Key changes to `app/src/main.c`

| Change | Before | After |
|--------|--------|-------|
| Timer setup | `main.c` overrode `vPortSetupTimerInterrupt()` with 32-bit `pullMachineTimerCompareRegister` | Removed override; `port.c` weak version used (64-bit pointer) |
| Extern declarations | `extern volatile uint32_t *pullMachineTimerCompareRegister;` etc. | Removed (provided by `port.c`) |
| LED blink | `LedBlinkTask` FreeRTOS task using `vTaskDelay()` | Polled `while(1)` loop using `timer_put_delay()` |
| LED pin | GPIO 0 | GPIO 24 (RGB LED red) |
| UART debug | None | `printf("LED ON\n\r")` / `printf("LED OFF\n\r")` |
| Timer header | Not included | `#include "timer.h"` |

## Code

```c
/* Blink RGB LED red on GPIO pin 24 using polled delay (no scheduler tick needed) */
while (1)
    {
      GPIO_write_pin (24, ON_LED);
      printf ("LED ON\n\r");
      timer_put_delay (TIMER_0, SYSTEM_FREQUENCY / 2);

      GPIO_write_pin (24, OFF_LED);
      printf ("LED OFF\n\r");
      timer_put_delay (TIMER_0, SYSTEM_FREQUENCY / 2);
    }
```

- `GPIO_write_pin(24, ON_LED)` — turns red LED on (active-low per `led.c`: `ON_LED_ARIES = 0`)
- `GPIO_write_pin(24, OFF_LED)` — turns red LED off (`OFF_LED_ARIES = 1`)
- `timer_put_delay(TIMER_0, SYSTEM_FREQUENCY / 2)` — polled delay of ~0.5s (100MHz / 2)
- `printf` statements confirm on/off state over UART (115200 baud)

## RGB LED Pin Mapping

| Color | GPIO Pin |
|-------|----------|
| Red   | GP24 |
| Green | GP22 |
| Blue  | GP23 |

Only the red channel (GP24) is used in the current blink loop.

## Serial Listener

A host-side C serial listener was written to `/tmp/kilo/serial_listener.c` and compiled to `/tmp/kilo/serial_listener`. It opens `/dev/ttyUSB0` at 115200 baud and prints UART output with timestamps:

```bash
gcc -o /tmp/kilo/serial_listener /tmp/kilo/serial_listener.c
timeout 30 /tmp/kilo/serial_listener /dev/ttyUSB0
```

## Build & Flash

```bash
# Build (cross-compiler at ~/vega-tools/bin)
make build

# Flash via xmodem (requires FLASH_TOOL at ~/Desktop/vega-flasher/xmodemflasher)
make flash

# Monitor
make monitor
# or: picocom --baud 115200 --flow n --echo /dev/ttyUSB0
```

Build output: `freertos-thejas32/build/RTOSDemo.elf` / `.bin` / `.hex`

## Verification

- `make build` succeeds (no new warnings beyond pre-existing ones)
- Serial listener confirms `LED ON` / `LED OFF` print continuously
- No automatic reset after `hiii 5` — the board runs continuously

## Navigation

- [[Developer Notes]] — Project overview and phases
- [[API Reference]] — GPIO, timer, UART, LED headers
- [[Flash Workflow]] — Flashing to SRAM vs SPI FLASH
- [[Makefile Evolution]] — How the Makefile grows across phases
- [[Makefile Audit + Proposed HAL-OS Architecture]] — Makefile audit and proposed HAL/OS layers

All notes link back to [[_index.md]].
