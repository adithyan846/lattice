# Porting guide

## 1. New board checklist

1. Create `boards/<name>/`.
2. Add `board.h`, `board.c`, `pinmap.h`, `clock.c`.
3. Implement the HAL interfaces from `src/hal/include/hal/` in
   `boards/<name>/hal/`.
4. Add `ld/link.ld` and `startup.S`.
5. Add `config/boards/<name>/default.params` and `init.d/rcS`.
6. Add the board to `cmake/options.cmake` validation and the Makefile.
7. Build with `-DFREERTRTOS_BOARD=<name>` and a matching toolchain file.
8. Bring up in this order: UART console → clock → GPIO → IMU → PWM (no props) →
   RC → telemetry → GNSS → SD.

## 2. HAL contracts

All board implementations must satisfy the headers in `src/hal/include/hal/`:

| Interface | Required semantics |
|---|---|
| `hal_uart` | Non-blocking read with timeout, blocking write with timeout |
| `hal_spi` | Full-duplex transfer, chip-select handled by caller or bus |
| `hal_i2c` | 7-bit address, repeated-start supported |
| `hal_gpio` | Input/output, pull-up/down, interrupt on edge |
| `hal_pwm` | 4-channel ESC output, 1 µs resolution, safe-off at init |
| `hal_adc` | Battery voltage/current channels, calibrated |
| `hal_flash` | Erase/write/read parameter and mission partitions |
| `hal_sd` | Block read/write for the logger filesystem |

The HAL must **never** block interrupts for more than a few microseconds.
Long transfers belong in a task, not an ISR.

## 3. freertrtos port

`os/freertrtos/` is an adaptation layer, not a new kernel. It provides:

- `FreeRTOSConfig.h` for the board,
- typed wrappers (`frt_task_create`, `frt_queue_send`, `frt_time_us`),
- boot-time kernel object creation,
- hooks for stack overflow, malloc failure and assert.

To port to a new core:

1. Take the upstream FreeRTOS port for that ISA.
2. Copy its `port.c` / `portASM.S` into `third_party/<port>/`.
3. Copy `os/freertrtos/port/thejas32/FreeRTOSConfig.h` and adjust:
   - `configCPU_CLOCK_HZ`
   - `configTICK_RATE_HZ` (keep 1000 unless tickless idle is required)
   - `configTOTAL_HEAP_SIZE`
   - `configMAX_SYSCALL_INTERRUPT_PRIORITY`
   - `configUSE_FPU` if applicable
4. Verify `configASSERT` maps to a visible fault (LED + UART + reboot).
5. Measure the tick interrupt cost and the context-switch latency.

Do not mix a port written for one privilege mode with a kernel configured for
another. On RISC-V, confirm machine-mode trap setup and the `ecall` yield path.

## 4. Driver porting

A driver has three parts:

```
device register access  →  bus HAL (SPI/I2C/UART)
device data conversion  →  driver .c
publication             →  uORB topic
```

Keep register definitions in the driver, not in the board. Keep pin/chip-select
definitions in the board `pinmap.h`. A driver must be testable with a mock bus
and recorded register/data streams.

## 5. Board bring-up tests

| Step | Test | Pass criterion |
|---|---|---|
| 1 | UART shell | Boot banner, `status` command |
| 2 | Clock | `hrt` advances at the expected rate over 60 s |
| 3 | GPIO | LED toggles, safety switch reads correctly |
| 4 | IMU | Stationary gyro < 0.5 °/s, accel ≈ 1 g |
| 5 | Baro | Pressure matches a reference within 2 hPa |
| 6 | Mag | Heading within 5° of a known reference |
| 7 | PWM | 1000–2000 µs on all four outputs, props removed |
| 8 | RC | All channels move smoothly, failsafe bit correct |
| 9 | Telemetry | Heartbeat and status in QGC |
| 10 | GNSS | 3D fix, satellite count, position on map |
| 11 | SD | Log file created, readable after reboot |
| 12 | Watchdog | Blocked task triggers failsafe |

## 6. Interrupt priorities

- SysTick and the context-switch interrupt must be at the lowest priority
  allowed to call FreeRTOS APIs.
- IMU data-ready may be higher; it only posts a notification.
- UART RX may be higher; it pushes bytes into a ring buffer.
- Never call a FreeRTOS `FromISR` API from an interrupt above
  `configMAX_SYSCALL_INTERRUPT_PRIORITY`.
- Document every interrupt priority in `boards/<name>/pinmap.h` or a board
  table.

## 7. Memory and timing

- Place hot control data in fast RAM if the SoC has it.
- Keep the FreeRTOS heap out of the control path; use static allocation.
- Measure worst-case stack high-water marks over a full mission.
- Toggle a GPIO at the start/end of each control task during bring-up and
  measure with a scope.
