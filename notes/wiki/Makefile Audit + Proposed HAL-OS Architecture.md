# Makefile Audit + Proposed HAL/OS Architecture

---
tags: [makefile, hal, architecture, os-abstraction, audit]
status: active
created: 2026-11-02
updated: 2026-11-02
---

# Makefile Audit + Proposed HAL/OS Architecture

> Audit of the root `Makefile` changes and the proposed HAL/RTOS abstraction layer architecture for Lattice.

## Makefile Audit

The root `Makefile` is a complete rewrite of the build system. Key changes vs. the documented Phase 0 (submodule-only) approach:

### Build System

| Aspect | Before (Phase 0) | After (Current) |
|--------|-------------------|-----------------|
| Root Makefile | None — used submodule directly | Single `Makefile` at repo root |
| Toolchain | `CROSS_COMPILE ?= riscv32-vega-elf` | Same, with `VEGA_TOOLS ?= ~/vega-tools` |
| Architecture | `-march=rv32im_zicsr -mabi=ilp32 -mcmodel=medany` | Same |
| Sources | Manual `APP_SRCS` | Auto-discovered from `app/src/*.c` only |
| Build dir | `freertos-thejas32/build` | `freertos-thejas32/build` |
| Target name | `RTOSDemo` | `RTOSDemo` |

### Targets

| Target | Description |
|--------|-------------|
| `make` / `make build` | Build demo + app |
| `make clean` | Remove build artifacts |
| `make flash` | Flash `.bin` via xmodem (tool: `~/Desktop/vega-flasher/xmodemflasher`) |
| `make run` | Build + flash |
| `make monitor` | Flash + open picocom serial console |
| `make size` | Print ELF section sizes |
| `make disasm` | Regenerate disassembly listing |
| `make info` | Print resolved toolchain / board config |
| `make check-serial` | Verify serial port present and accessible |

### Key Features

- **Toolchain validation**: `check-toolchain` actually compiles a test file (not just `--version`) to detect `libisl.so.22` missing
- **Serial port pre-flight**: `check-serial` verifies read/write access, suggests `sudo usermod -aG dialout`
- **libisl shim**: `TOOLCHAIN_LIB ?= ~/vega-tools/lib` added to `LD_LIBRARY_PATH` for gcc 13.x
- **BSP include ordering**: BSP's own `stdlib.h`/`printf` shadow newlib (exact Eclipse order)
- **No `-nostdlib`**: newlib supplies crt0 glue; `-nostartfiles` keeps crt0.o out
- **`-fno-builtin-*`**: prevents gcc from turning BSP's own `memset`/`memcpy`/`printf` into self-referential calls
- **`configISR_STACK_SIZE_WORDS = 4096`** (set in FreeRTOSConfig.h)

### Issues Found

1. **`hal/thejas32/.gitkeep` deleted** — directory structure changed; `hal/` now empty at root level
2. **README.md updated** — reflects new build system
3. **CMakeLists.txt added** — IDE support (VS Code, CLion) but references `app/src/main.c` as separate target
4. **`app/Makefile` added** — separate app build (unclear if used)
5. **`.gitignore` added** — not previously present
6. **`notes/` directory** — Obsidian vault (knowledge base)

## Proposed HAL/OS Architecture

### Motivation

FreeRTOS APIs are directly coupled to `app/src/main.c` (e.g., `xTaskCreate`, `vTaskDelay`, `pdMS_TO_TICKS`). This spreads dependency throughout Lattice. The proposed solution introduces two abstraction layers:

### Layer 1: `core/os` — RTOS Abstraction

```
lattice/
├── core/
│   ├── os/
│   │   ├── include/
│   │   │   ├── os_task.h
│   │   │   ├── os_mutex.h
│   │   │   ├── os_queue.h
│   │   │   └── os_time.h
│   │   └── src/
│   │       ├── os_task.c
│   │       ├── os_mutex.c
│   │       ├── os_queue.c
│   │       └── os_time.c
```

**Dependency chain:**
```
Lattice subsystems → core/os → FreeRTOS implementation → Hardware
```

**Example:**
```c
// Application uses:
os_task_create(sensor_task, "sensor", 512, NULL, OS_PRIORITY_NORMAL);

// Implementation translates to:
xTaskCreate(sensor_task, "sensor", 512, NULL, tskIDLE_PRIORITY + 2, NULL);
```

**What to abstract vs. keep:**

| FreeRTOS Feature | Recommendation |
|-----------------|----------------|
| Tasks | Abstract |
| Queues | Abstract |
| Mutexes | Abstract |
| Semaphores | Abstract |
| Timing/delays | Abstract |
| Event groups | Abstract when needed |
| Stream buffers | Abstract when needed |
| Scheduler internals | Keep in RTOS integration layer |

### Layer 2: `hal/` + `platform/` — Hardware Adaptation

```
lattice/
├── hal/
│   ├── include/
│   │   ├── hal_gpio.h
│   │   ├── hal_i2c.h
│   │   ├── hal_spi.h
│   │   ├── hal_uart.h
│   │   ├── hal_pwm.h
│   │   └── hal_timer.h
│   └── src/
│
├── platform/
│   └── thejas32/
│       ├── gpio/
│       ├── i2c/
│       ├── spi/
│       ├── uart/
│       ├── pwm/
│       └── timer/
```

**Responsibility split:**

| Layer | Responsibility |
|-------|----------------|
| `interfaces/` | Defines contracts and data types |
| `hal/` | Provides hardware-facing APIs (headers only) |
| `platform/` | Translates Lattice HAL APIs into SDK calls |
| Vendor BSP | Performs actual register-level operations |
| `devices/` | Implements sensor/actuator behavior |
| `core/` | Coordinates system behavior |

**Rule:** Never expose vendor SDK types in public Lattice HAL headers. The platform adapter is the only place that includes vendor BSP headers.

### Key Architectural Rule

> Application and subsystem code should depend on Lattice-owned interfaces, not directly on vendor BSP or FreeRTOS APIs.

**Exception:** FreeRTOS-specific code may exist in the integration layer (task startup, interrupt integration, scheduler initialization).

### Current Status

- `app/src/main.c` currently uses FreeRTOS APIs directly (`xTaskCreate`, `vTaskDelay`, `pdMS_TO_TICKS`, `vTaskStartScheduler`) and BSP headers (`uart.h`, `gpio.h`, `timer.h`, `config.h`)
- The proposed `core/os` and `hal/` + `platform/` layers are **not yet implemented**
- The LED blinking workaround (polled timer, no scheduler) avoids the FreeRTOS timer-interrupt reset but does not use the abstraction layer

## Navigation

- [[Developer Notes]] — Project overview and phases
- [[Architecture Details]] — Detailed layer specifications
- [[Makefile Evolution]] — How the Makefile grows across phases
- [[Flash Workflow]] — Flashing to SRAM vs SPI FLASH
- [[API Reference]] — All available headers and functions
- [[LED Blinking]] — LED blinking implementation

All notes link back to [[_index.md]].
