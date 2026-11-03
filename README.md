# Lattice - FreeRTOS on THEJAS32 (ARIES)

**Clone → Build → Flash → Done. No IDE, no Docker, no config files.**

## Quick Start

```bash
# 1. Clone with submodules
git clone --recurse-submodules <your-repo-url>
cd lattice

# 2. Initialize (if not cloned with --recurse-submodules)
make init

# 3. Build FreeRTOS demo (bare-metal SPI HAL)
make

# 4. Build with FreeRTOS SPI HAL (mutex + timeout)
make WITH_FREERTOS=1

# 5. Flash to board
make flash

# 6. Open serial monitor (Ctrl-A Ctrl-Q to quit)
make monitor
```

## Build Modes

| Mode | Command | SPI HAL | RAM | FreeRTOS |
|------|---------|---------|-----|----------|
| **Bare-metal** (default) | `make` | `spi_baremetal.c` | 68 KB | ❌ |
| **FreeRTOS** | `make WITH_FREERTOS=1` | `spi_freertos.c` | 90 KB | ✅ Mutex + timeout |

## Build Your Own App

```bash
# Edit your code
vim app/src/main.c

# Build app (bare-metal SPI)
make app

# Build app with FreeRTOS SPI
make WITH_FREERTOS=1 app

# Flash your app
make flash-app

# Monitor
make monitor
```

## HAL APIs in `hal/include/`

| Category | Header | Key Functions |
|----------|--------|---------------|
| **SPI** | `hal_spi.h` | `hal_spi_init`, `hal_spi_transfer`, `hal_spi_write_read`, `hal_spi_write`, `hal_spi_read`, `hal_spi_set_cs`, `hal_spi_set_baud_div`, `hal_spi_set_mode`, `hal_spi_check_overrun` |
| **GPIO** | `hal_gpio.h` | `hal_gpio_init`, `hal_gpio_write`, `hal_gpio_read`, `hal_gpio_toggle`, `hal_gpio_set_dir` |
| **UART** | `hal_uart.h` | `hal_uart_init`, `hal_uart_write`, `hal_uart_read`, `hal_uart_deinit` |
| **Timer** | `hal_timer.h` | `hal_timer_delay_ms` |
| **FreeRTOS Kernel** | `FreeRTOS.h`, `task.h`, `queue.h`, `semphr.h`, `timers.h`, `event_groups.h`, `stream_buffer.h` | `xTaskCreate`, `xQueueCreate`, `xSemaphoreCreateMutex`, `xTimerCreate`, `xEventGroupCreate`, `xStreamBufferCreate` |

## SPI HAL — Key Features

```c
// Config (both builds)
typedef enum { HAL_SPI_BAUD_DIV_4, ..., HAL_SPI_BAUD_DIV_2048 } hal_spi_baud_div_t;
typedef enum { HAL_SPI_CS_MODE_CSAAT, HAL_SPI_CS_MODE_GPIO, HAL_SPI_CS_MODE_NONE } hal_spi_cs_mode_t;

typedef struct {
    hal_spi_mode_t       mode;
    hal_spi_bit_order_t  bit_order;
    hal_spi_cs_mode_t    cs_mode;
    union { uint8_t gpio_pin; hal_spi_cs_hw_t hw_cs; };
    uint32_t             freq_hz;       // 0 = use baud_div
    hal_spi_baud_div_t   baud_div;      // Explicit divider
    uint8_t              word_size;     // 8 only
} hal_spi_config_t;

// All blocking calls accept timeout_ms (0 = infinite)
int hal_spi_init(hal_spi_bus_t bus, const hal_spi_config_t *config);
int hal_spi_transfer(hal_spi_bus_t bus, const uint8_t *tx, uint8_t *rx, uint32_t len, uint32_t timeout_ms);
int hal_spi_write_read(hal_spi_bus_t bus, const uint8_t *tx, uint32_t tx_len, uint8_t *rx, uint32_t rx_len, uint32_t timeout_ms);
int hal_spi_write(hal_spi_bus_t bus, const uint8_t *data, uint32_t len, uint32_t timeout_ms);
int hal_spi_read(hal_spi_bus_t bus, uint8_t *data, uint32_t len, uint32_t timeout_ms);
int hal_spi_set_cs(hal_spi_bus_t bus, bool assert);
int hal_spi_set_baud_div(hal_spi_bus_t bus, hal_spi_baud_div_t div);
int hal_spi_set_mode(hal_spi_bus_t bus, hal_spi_mode_t mode);
int hal_spi_check_overrun(hal_spi_bus_t bus);
```

## Make Targets

| Command | Description |
|---------|-------------|
| `make` / `make demo` | Build FreeRTOS demo (bare-metal SPI) |
| `make WITH_FREERTOS=1` | Build with FreeRTOS SPI HAL (mutex + timeout) |
| `make app` | Build your app from `app/src/` |
| `make WITH_FREERTOS=1 app` | Build app with FreeRTOS SPI HAL |
| `make flash` / `make flash-app` | Flash via xmodem |
| `make monitor` | Open picocom serial console |
| `make size` / `make size-app` | Show memory usage |
| `make clean` | Remove all build artifacts |
| `make info` | Show build configuration |
| `make help` | Show all targets |

## Override Variables

```bash
# Toolchain location
make VEGA_TOOLS=/opt/vega-tools
make TOOLCHAIN_BIN=/opt/vega-tools/bin

# Build mode
make                    # Bare-metal SPI HAL
make WITH_FREERTOS=1    # FreeRTOS SPI HAL (mutex + timeout)

# Serial/flasher
make SERIAL=/dev/ttyACM0
make FLASHER=/usr/local/bin/xmodemflasher

# Build type
make OPT=-O2            # Release
make OPT="-Og -g"       # Debug (default)
```

## Project Structure

```
lattice/
├── freertos-thejas32/              # FreeRTOS port (git submodule)
│   ├── FreeRTOS_Source/            # Kernel source
│   └── Demo/RISCV_CDAC_THEJAS32/   # Demo + VEGA BSP
├── app/
│   └── src/                        # YOUR CODE HERE
│       └── main.c                  # Entry point
├── hal/
│   └── include/                    # Portable HAL headers
│       ├── hal_spi.h
│       ├── hal_gpio.h
│       ├── hal_uart.h
│       └── hal_timer.h
├── platform/thejas32/
│   ├── spi_baremetal.c             # SPI HAL (no RTOS)
│   ├── spi_freertos.c              # SPI HAL (FreeRTOS mutex + timeout)
│   ├── gpio.c
│   ├── uart.c
│   ├── timer.c
│   └── board/
├── notes/
│   └── embedded/spi/               # SPI HAL documentation
│       ├── 01-bsp-structure.md
│       ├── 02-thejas32-spi-hardware.md
│       ├── 03-vega-bsp-implementation.md
│       ├── 04-hal-design.md
│       ├── 05-platform-implementation.md
│       ├── debugging-report.md
│       ├── freertos-rationale.md
│       └── hal-bsp-analysis.md
├── Makefile                        # Single build script
└── README.md
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

## Reproducing on Another Machine

```bash
git clone --recurse-submodules <repo>
cd lattice
make init        # Only if submodules not cloned
make             # Build demo (bare-metal SPI)
make flash       # Flash
make monitor     # See output
```

That's it. One Makefile, standard tools, portable HAL.

## License

FreeRTOS: MIT (see `freertos-thejas32/FreeRTOS_Source/LICENSE.md`)
BSP/HAL: CDAC VEGA SDK licenses
This build system: Use freely