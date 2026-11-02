# _index.md

---
tags: [index, architecture, makefile]
status: active
created: 2026-11-01
---

# Lattice Flight Controller — Developer Knowledge Base

> Flight Controller on THEJAS32 (ARIES) RISC-V using FreeRTOS
> Platform: CDAC VEGA Mi-V soft processor

## Table of Contents

- [[Developer Notes]]
- [[Architecture Details]]
- [[Makefile Evolution]]
- [[Flash Workflow]]
- [[API Reference]]
- [[LED Blinking]]
- [[Makefile Audit + Proposed HAL-OS Architecture]]

## Project Structure

```
lattice/
├── freertos-thejas32/          # Git submodule (platform + HAL + RTOS)
├── interfaces/                 # Abstract APIs (stable, portable)
├── devices/                    # Concrete drivers (implement interfaces)
├── core/                       # Pure algorithms (host-testable)
├── app/                        # Flight logic
├── hal/                        # Wrappers (only when porting)
├── config/                     # Build-time driver selection
├── Makefile                    # Single root build script
├── README.md
├── CLAUDE.md
└── _index.md
```

## Quick Start

1. **Initialize submodules**: `git submodule update --init --recursive`
2. **Build demo**: `make -C freertos-thejas32 CROSS_COMPILE=riscv32-vega-elf RISCV_ARCH=rv32im_zicsr`
3. **Flash to SRAM (dev)**: `/home/siesta/flasher/vega-flasher/vegaxmodem /dev/ttyUSB0 freertos-thejas32/build/RTOSDemo.elf`
4. **Flash to SPI (prod)**: `/home/siesta/flasher/vega-flasher/flasher.bat freertos-thejas32/build/RTOSDemo.elf`
5. **Monitor**: `picocom --baud 115200 --flow n --echo /dev/ttyUSB0`

## Navigation

Use [[Double Bracket Links]] to navigate between notes. The Obsidian Graph View will show connections between:

- [[Developer Notes]] → [[Architecture Details]] → [[Makefile Evolution]] → [[Flash Workflow]] → [[API Reference]] → [[LED Blinking]] → [[Makefile Audit + Proposed HAL-OS Architecture]]

All notes link back to [[_index.md]].
