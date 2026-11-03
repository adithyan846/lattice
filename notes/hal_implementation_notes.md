# HAL Implementation Notes — UART Debug Fix

**Project:** Lattice Flight Controller (THEJAS32 / ARIES)
**Date:** 2026-11-02
**Status:** Resolved

## Problem

After implementing the HAL platform layer, UART output from `hal_uart_write()` was garbled at every baud rate:

```
M^ÕþM^tM^ÕþM^t...
```

Expected output: clean `LED ON` / `LED OFF` lines at 115200 baud.

The GPIO and Timer parts of the HAL worked correctly (LED blinked on GPIO pin 24), but UART was corrupted at the source — not just a receiver baud mismatch.

## Root Cause

`platform/thejas32/uart.c` called `init_uart()` from `debug_uart.c` during `hal_uart_init()`. That function hardcodes the baud divisor `0x15` for **115200 @ 40MHz UART clock**.

The board's **bootloader already initialized UART_0 at 115200** before jumping to the application, and printed its banner cleanly at that rate. Re-initializing the same UART (`UART_BASE = 0x10000100`, UART_0) with a hardcoded divisor overwrote the bootloader's correct configuration with a mismatched one, corrupting every transmitted byte.

## Fix

In `platform/thejas32/uart.c`, `hal_uart_init()` no longer calls `init_uart()`. It relies on the bootloader's pre-configured UART and uses `tx_uart()` / `rx_uart()` directly:

```c
int hal_uart_init(hal_uart_t uart, const hal_uart_config_t *config) {
    (void)config;
    if (uart != HAL_UART_0) return -ENOTSUP;
    /* UART_0 already initialized by bootloader at 115200; do not re-init */
    return 0;
}
```

## Validation

After the fix, pyserial on `/dev/ttyUSB2 @ 115200` produced clean output:

```
LED ON
LED OFF
LED ON
LED OFF
LED ON
LED OFF
LED ON
LED OFF
```

This validates the full HAL platform stack on hardware:
- **GPIO**: `hal_gpio_write()` → pin 24 blinks (LED confirmed working)
- **UART**: `hal_uart_write()` → clean text at 115200 baud
- **Timer**: `hal_timer_delay_ms()` → correct 500ms blink cadence

## Lessons Learned

- Never re-initialize a peripheral that the bootloader has already configured without verifying the clock/divisor matches.
- On THEJAS32, `UART_BASE = 0x10000100` (UART_0) is shared between `debug_uart` and BSP `uart.h`; `debug_uart` uses the same base as BSP UART_0.
- `SYSTEM_FREQUENCY` (100 MHz) is the CPU clock — not necessarily the UART peripheral clock. The bootloader's UART config is authoritative for the running board.

## Files Changed

- `platform/thejas32/uart.c` — removed `init_uart()` call; use bootloader's UART config
