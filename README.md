# Lattice - FreeRTOS on THEJAS32 (ARIES)

**Clone → Build → Flash → Done. No IDE, no Docker, no config files.**

## Quick Start

```bash
# 1. Clone with submodules
git clone --recurse-submodules <your-repo-url>
cd lattice

# 2. Initialize (if not cloned with --recurse-submodules)
make init

# 3. Build FreeRTOS demo
make

# 4. Flash to board
make flash

# 5. Open serial monitor (Ctrl-A Ctrl-Q to quit)
make monitor
```

## Build Your Own App

```bash
# Edit your code
vim app/src/main.c

# Build app (links against pre-built FreeRTOS kernel)
make app

# Flash your app
make flash-app

# Monitor
make monitor
```

## All Available APIs in `app/src/`

| Category | Headers | Examples |
|----------|---------|----------|
| **FreeRTOS Kernel** | `FreeRTOS.h`, `task.h`, `queue.h`, `semphr.h`, `timers.h`, `event_groups.h`, `stream_buffer.h`, `list.h`, `croutine.h` | `xTaskCreate`, `xQueueCreate`, `xSemaphoreCreateMutex`, `xTimerCreate`, `xEventGroupCreate`, `xStreamBufferCreate` |
| **Debug UART** | `debug_uart.h` | `init_uart()`, `Transmit_uart()`, `tx_uart()`, `rx_uart()`, `get_hex()`, `get_decimal()` |
| **UART Driver** | `uart.h` | `uart_init()`, `uart_putchar()`, `uart_getchar()`, `uart_configure()`, `uart_intr_enable()` |
| **GPIO** | `gpio.h` | `gpio_init()`, `gpio_set()`, `gpio_clear()`, `gpio_toggle()`, `gpio_read()`, `gpio_config()` |
| **LED** | `led.h` | `led_init()`, `vToggleLED()`, `led_on()`, `led_off()` |
| **I2C** | `i2c.h` | `i2c_init()`, `i2c_write()`, `i2c_read()`, `i2c_write_read()` |
| **SPI** | `spi.h` | `spi_init()`, `spi_transfer()`, `spi_write()`, `spi_read()` |
| **Timer** | `timer.h` | `timer_init()`, `timer_start()`, `timer_stop()`, `timer_get_count()` |
| **Interrupt** | `interrupt.h` | `interrupt_enable()`, `interrupt_disable()`, `interrupt_configure()`, `enable_irq()`, `disable_irq()` |
| **PWM** | `pwmc.h` | `pwmc_init()`, `pwmc_set_duty()`, `pwmc_set_period()`, `pwmc_start()`, `pwmc_stop()` |
| **ADC** | `adc.h` | `adc_init()`, `adc_read()`, `adc_config_channel()` |
| **QSPI** | `qspi.h` | `qspi_init()`, `qspi_read()`, `qspi_write()`, `qspi_erase()` |
| **Flash** | `at25sf161.h` | `at25sf161_init()`, `at25sf161_read()`, `at25sf161_write()`, `at25sf161_erase()` |

## Make Targets

| Command | Description |
|---------|-------------|
| `make` / `make demo` | Build FreeRTOS demo (RTOSDemo.elf/bin/hex) |
| `make app` | Build your app from `app/src/` |
| `make flash` | Flash demo binary via xmodem |
| `make flash-app` | Flash your app binary via xmodem |
| `make monitor` | Open picocom serial console |
| `make size` | Show demo memory usage |
| `make size-app` | Show app memory usage |
| `make clean` | Remove all build artifacts |
| `make info` | Show build configuration |
| `make help` | Show all targets |

## Override Variables

```bash
# Toolchain location
make VEGA_TOOLS=/opt/vega-tools
make TOOLCHAIN_BIN=/opt/vega-tools/bin

# Serial/flasher
make SERIAL=/dev/ttyACM0
make FLASHER=/usr/local/bin/xmodemflasher

# Build type
make OPT=-O2          # Release
make OPT="-Og -g"     # Debug (default)
```

## Requirements

1. **VEGA Toolchain** (`riscv32-vega-elf-gcc`)
   - Default: `~/vega-tools/bin`
   - Set `VEGA_TOOLS=/path` if elsewhere
   - Needs `libisl.so.22` (install isl-0.22 if missing)

2. **xmodemflasher**
   - Default: `~/Desktop/vega-flasher/xmodemflasher`
   - Set `FLASHER=/path` if elsewhere

3. **Serial Access**
   - Board typically: `/dev/ttyUSB0`
   - Add user to dialout: `sudo usermod -aG dialout $USER` (logout/login)

4. **picocom** (for monitor)
   - `sudo apt install picocom`

## Project Structure

```
lattice/
├── freertos-thejas32/          # FreeRTOS port (git submodule)
│   ├── FreeRTOS_Source/        # Kernel source
│   └── Demo/RISCV_CDAC_THEJAS32/  # Demo + HAL/BSP
├── app/
│   └── src/                    # YOUR CODE HERE
│       └── main.c              # Entry point
├── Makefile                    # Single build script (this is it)
└── README.md
```

## Reproducing on Another Machine

```bash
git clone --recurse-submodules <repo>
cd lattice
make init        # Only if submodules not cloned
make             # Build demo
make flash       # Flash
make monitor     # See output
```

That's it. One Makefile, standard tools, all APIs available.

## License

FreeRTOS: MIT (see `freertos-thejas32/FreeRTOS_Source/LICENSE.md`)
BSP/HAL: CDAC VEGA SDK licenses
This build system: Use freely
