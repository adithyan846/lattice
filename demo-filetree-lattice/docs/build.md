# Build guide

## 1. Host unit tests

No hardware, kernel or cross-compiler is required:

```sh
cd /home/adithyan/demo-filetree-lattice
make test
```

This builds `tests/host/` with the host `cc` and runs `ctest`. The tests cover
the pure libraries (math, geo, mission, PID). They are the fastest way to
validate changes before touching firmware.

## 2. Firmware prerequisites

| Tool | Version | Notes |
|---|---|---|
| CMake | ≥ 3.20 | `cmake --version` |
| RISC-V GCC | any `riscv32-unknown-elf-gcc` | Set `TOOLCHAIN_PREFIX` |
| Python | 3.8+ | Message generator, log tools |
| FreeRTOS-Kernel | 10.5+ | `third_party/FreeRTOS-Kernel` |
| THEJAS32 port | matching kernel | `third_party/freertos-thejas32` |
| MAVLink C library | optional | `third_party/mavlink` |

### Get the kernel and port

```sh
cd third_party
git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git
git clone https://gitlab.com/riscv-vega/operating-systems/freertos-thejas32.git
```

If your port checkout has a different layout, point CMake at the directory that
contains `port.c`:

```sh
cmake -S . -B build/thejas32 \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-thejas32.cmake \
  -DFREERTRTOS_PORT_DIR=/path/to/port
```

### Toolchain

The reference toolchain is `riscv32-unknown-elf-`. Override the prefix or ISA:

```sh
cmake -S . -B build/thejas32 \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-thejas32.cmake \
  -DTOOLCHAIN_PREFIX=riscv64-unknown-elf- \
  -DFREERTRTOS_ARCH=rv32imac \
  -DFREERTRTOS_ABI=ilp32
```

If your core has an FPU, add `-DFREERTRTOS_USE_FPU=ON` and adjust
`FreeRTOSConfig.h` and the port context-save code together. Do not enable the
FPU in software and the port independently.

## 3. Build

```sh
make thejas32
```

Artifacts:

```
build/thejas32/freertrtos_autopilot.elf
build/thejas32/freertrtos_autopilot.map
```

Convert to raw binary:

```sh
riscv32-unknown-elf-objcopy -O binary \
  build/thejas32/freertrtos_autopilot.elf \
  build/thejas32/freertrtos_autopilot.bin
```

## 4. Memory map

The reference linker script is `boards/thejas32/ld/link.ld`. It defines:

| Region | Purpose |
|---|---|
| `FLASH` | Vector table, `.text`, `.rodata` |
| `RAM` | `.data`, `.bss`, FreeRTOS heap, main stack |
| `CCM` (optional) | Fast scratch for estimator/control (if the SoC has it) |

Adjust the origins and lengths to the actual THEJAS32 board. Confirm with the
SoC datasheet, not with a guess. The linker script contains `ASSERT` statements
so an overflowing image fails at link time.

## 5. Flash

`tools/flash.sh` is a template. Fill in the programmer command for your board
(OpenOCD, J-Link, `dfu-util`, vendor tool):

```sh
./tools/flash.sh build/thejas32/freertrtos_autopilot.elf
```

Never flash a board with props attached. Verify the image hash and keep a known
good image for recovery.

## 6. Configuration

- Default parameters: `config/boards/thejas32/default.params`
- Boot script: `config/boards/thejas32/init.d/rcS`
- Airframe: `config/airframes/quad_x/`
- Mission: `config/missions/`

The build embeds the board defaults; the SD card may override them. Parameter
changes are persisted to flash only on an explicit `param save`.

## 7. Troubleshooting

| Symptom | Likely cause |
|---|---|
| `FreeRTOS kernel not found` | `third_party/FreeRTOS-Kernel` missing or `-DFREERTRTOS_KERNEL_DIR` wrong |
| `Could not find port.c` | Port checkout layout differs; set `FREERTRTOS_PORT_DIR` to the folder containing `port.c` |
| `undefined reference to _start` | Startup file not in the board sources, or linker script missing |
| Immediate reset after boot | Stack overflow, bad vector table, or clock configuration |
| `configASSERT` loop | Invalid FreeRTOS object or interrupt priority configuration |
| No PWM output | Timer/pin map, arming state, or safety switch |
| No telemetry | UART pins/baud, MAVLink system ID, or GCS baud |
| Watchdog reboot in flight | Task overrun or an ISR starving the scheduler |

## 8. Reproducible builds

- Pin the kernel and port commits.
- Record `git describe --dirty` in `src/main/version.h` at build time.
- Keep the toolchain version in CI logs.
- Archive the `.elf`, `.map`, `.bin` and parameter set for every flight.
