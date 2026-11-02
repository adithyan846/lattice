# Developer Notes

---
tags: [index, overview, phase-0]
status: active
created: 2026-11-01
---

# Developer Notes

> Project overview, directory layout, and development phases for the Lattice Flight Controller.

## Project Overview

The Lattice Flight Controller is a FreeRTOS-based flight control system running on the CDAC VEGA Mi-V soft processor (THEJAS32 / ARIES RISC-V).

**Toolchain:** `riscv32-vega-elf-gcc`
**Bootloader:** `vegaxmodem` (SRAM, volatile) / `flasher.bat` (SPI FLASH, persistent)
**RTOS:** FreeRTOS with RISC-V port

## Directory Layout

```
lattice/
├── freertos-thejas32/          # Git submodule — platform + HAL + RTOS
│   ├── FreeRTOS_Source/        # Kernel source
│   ├── Demo/RISCV_CDAC_THEJAS32/
│   │   ├── riscv-thejas32.ld   # Linker script (RAM @ 0x200000)
│   │   ├── thejas32_hal/
│   │   │   ├── riscv_hal/entry.S
│   │   │   └── drivers/        # BSP: GPIO, UART, I2C, SPI, PWM, Timer
│   │   └── build/RTOSDemo.elf  # Pre-built demo
│   └── build/
├── interfaces/                 # Abstract APIs (stable, portable)
├── devices/                    # Concrete drivers (implement interfaces)
├── core/                       # Pure algorithms (host-testable)
├── app/                        # Flight logic
├── hal/                        # Wrappers (ONLY when porting — YAGNI)
├── config/                     # Build-time driver selection
├── Makefile                    # Single root build script
├── README.md
├── CLAUDE.md
└── _index.md
```

## Development Phases

```mermaid
gantt
    title Flight Controller Development Timeline
    dateFormat  D-M-YYYY
    section Bring-up
    Test vendor BSP          :a1, 1d, 0-1
    UART + LED + I2C scan    :a2, 1d, 1-2
    section Interfaces
    IMU / Baro / GNSS / ESC  :b1, 2d, 2-4
    section Drivers
    ICM-42688 + BMP280       :c1, 2d, 4-6
    section Core Algorithms
    EKF + PID + AHRS         :d1, 3d, 6-9
    Host unit tests          :d2, 1d, 9-10
    section App Modules
    Flight / Mission / Modes :e1, 3d, 10-13
    section Build System
    Root Makefile evolution  :f1, 2d, 11-13
```

## Key Principles

1. **Vendor BSP for hardware access** — Don't rewrite GPIO/UART/I2C from `freertos-thejas32`
2. **Interfaces for architecture** — `interfaces/sensor/imu.h` stays stable across ports
3. **Core algorithms pure C** — Testable on host without hardware
4. **HAL wrappers only when porting** — YAGNI principle
5. **One Makefile** — Single root file, auto-discovers sources

## Navigation

- [[Architecture Details]] — Detailed layer specifications
- [[Makefile Evolution]] — How the Makefile grows across phases
- [[Flash Workflow]] — Flashing to SRAM vs SPI FLASH
- [[API Reference]] — All available headers and functions
- [[LED Blinking]] — LED blinking implementation and reset fix
- [[Makefile Audit + Proposed HAL-OS Architecture]] — Makefile audit and proposed HAL/OS abstraction layers

All notes link back to [[_index.md]].
