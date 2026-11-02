# Makefile Evolution

---
tags: [makefile, build, phase-1, phase-2, phase-3, phase-4, phase-5]
status: active
created: 2026-11-01
---

# Makefile Evolution

> Tracks how the root `Makefile` grows incrementally across development phases.

## Phase 0: Baseline (Day 0)

**No root Makefile.** Use submodule directly:

```bash
cd freertos-thejas32
make CROSS_COMPILE=riscv32-vega-elf RISCV_ARCH=rv32im_zicsr
# Output: freertos-thejas32/build/RTOSDemo.elf
```

Flash separately:
```bash
/home/siesta/flasher/vega-flasher/vegaxmodem /dev/ttyUSB0 freertos-thejas32/build/RTOSDemo.elf
```

---

## Phase 1: Bring-up (Day 1)

**File:** `Makefile` v1

```makefile
VENDOR := freertos-thejas32
VENDOR_BUILD := $(VENDOR)/build
VENDOR_INC := $(VENDOR)/Demo/RISCV_CDAC_THEJAS32/thejas32_hal/drivers/vega-sdk_freertos_thejas32/bsp/include
VENDOR_DEMO := $(VENDOR)/Demo/RISCV_CDAC_THEJAS32

TOOLCHAIN ?= $(HOME)/vega-tools/bin
PREFIX    ?= riscv32-vega-elf
CC        := $(TOOLCHAIN)/$(PREFIX)-gcc
OBJCOPY   := $(TOOLCHAIN)/$(PREFIX)-objcopy

ARCH  := -march=rv32im_zicsr -mabi=ilp32 -mcmodel=medany
CFLAGS := $(ARCH) -Wall -Wextra -Og -g -std=gnu11 -ffunction-sections -fdata-sections -MMD -MP
LDFLAGS := $(ARCH) -T$(VENDOR_DEMO)/riscv-thejas32.ld -nostartfiles -Wl,--gc-sections
LDLIBS  := -Wl,--start-group -lgcc -lc -lm -Wl,--end-group

INCLUDES := -I$(VENDOR_INC) -I$(VENDOR_DEMO) -Iapp/src

APP_SRCS := $(wildcard app/src/*.c)
APP_OBJS := $(addprefix build/,$(APP_SRCS:.c=.o))
VENDOR_OBJS := $(wildcard $(VENDOR_BUILD)/*.o)

.PHONY: all flash-sram flash-spi monitor clean

all: build/firmware.elf

build/firmware.elf: $(APP_OBJS) $(VENDOR_OBJS) $(VENDOR_DEMO)/riscv-thejas32.ld
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(APP_OBJS) $(VENDOR_OBJS) $(LDLIBS) -o $@

build/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

flash-sram: all
	/home/siesta/flasher/vega-flasher/vegaxmodem /dev/ttyUSB0 $<

flash-spi: all
	/home/siesta/flasher/vega-flasher/flasher.bat $<

monitor:
	picocom --baud 115200 --flow n --echo /dev/ttyUSB0

clean:
	rm -rf build
```

**Changes from Phase 0:**
- Root Makefile exists
- Links your `app/src/*.c` against vendor BSP objects
- `flash-sram` / `flash-spi` targets

---

## Phase 2: Interfaces (Day 2-3)

**File:** `Makefile` v2

**Change:** Add `-Iinterfaces` to include path:

```makefile
INCLUDES := -Iinterfaces -I$(VENDOR_INC) -I$(VENDOR_DEMO) -Iapp/src
```

That's it. No source changes needed yet because `interfaces/` has only headers.

---

## Phase 3: Drivers (Day 3-5)

**File:** `Makefile` v3

**Change:** Auto-discover sources instead of manual `APP_SRCS`:

```makefile
YOUR_DIRS := app core devices interfaces config
YOUR_SRCS := $(foreach d,$(YOUR_DIRS),$(wildcard $(d)/**/*.c) $(wildcard $(d)/*.c))
YOUR_OBJS := $(addprefix build/,$(YOUR_SRCS:.c=.o))

INCLUDES := -Iinterfaces -Icore -Idevices -Iconfig -Iapp -I$(VENDOR_INC) -I$(VENDOR_DEMO)

build/firmware.elf: $(YOUR_OBJS) $(VENDOR_OBJS) $(VENDOR_DEMO)/riscv-thejas32.ld
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(YOUR_OBJS) $(VENDOR_OBJS) $(LDLIBS) -o $@

build/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
```

**Changes:**
- `YOUR_SRCS` auto-discovers all `.c` in your layers
- `INCLUDES` adds `-Icore -Idevices -Iconfig`

---

## Phase 4: Core Algorithms + Host Tests (Day 5-10)

**File:** `Makefile` v4

**Change:** Add host test build target:

```makefile
HOST_CC := gcc
HOST_CFLAGS := -Wall -Wextra -Og -g -Icore -Iinterfaces -Idevices
TEST_SRCS := $(wildcard tests/unit/*.c)
TEST_OBJS := $(addprefix build/host/,$(TEST_SRCS:.c:.o))

.PHONY: test-host
test-host: build/host/test_ekf
	@./build/host/test_ekf

build/host/test_ekf: build/host/tests/unit/test_ekf.o build/host/core/estimation/ekf.o
	$(HOST_CC) $^ -o $@ -lm

build/host/%.o: %.c
	@mkdir -p $(@D)
	$(HOST_CC) $(HOST_CFLAGS) -c $< -o $@
```

**Changes:**
- Host compiler (`gcc`) separate from cross-compiler
- `test-host` target runs unit tests without target hardware
- Core algorithms compiled natively on Linux

---

## Phase 5: App Modules (Day 10-15)

**File:** `Makefile` v5

**Change:** Add FreeRTOS kernel includes:

```makefile
FREERTOS_INC := $(VENDOR)/FreeRTOS_Source/include
FREERTOS_PORT := $(VENDOR)/FreeRTOS_Source/portable/GCC/RISC-V

INCLUDES := -Iinterfaces -Icore -Idevices -Iconfig -Iapp \
            -I$(FREERTOS_INC) -I$(FREERTOS_PORT) \
            -I$(VENDOR_INC) -I$(VENDOR_DEMO)
```

---

## Phase 6: Production Polish (Day 15+)

**File:** `Makefile` v5 (complete)

Add binary/hex/map/dump generation:

```makefile
ELF := build/firmware.elf
BIN := build/firmware.bin
HEX := build/firmware.hex
MAP := build/firmware.map

$(ELF): $(YOUR_OBJS) $(VENDOR_OBJS) $(VENDOR_DEMO)/riscv-thejas32.ld
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(YOUR_OBJS) $(VENDOR_OBJS) $(LDLIBS) -o $@ -Wl,-Map=$(MAP)
	$(SIZE) $@
	$(OBJDUMP) -d $@ > $(ELF:.elf=.dump)

$(BIN): $(ELF); $(OBJCOPY) -I elf32-littleriscv -O binary $< $@
$(HEX): $(ELF); $(OBJCOPY) -I elf32-littleriscv -O ihex $< $@
```

---

## Makefile Version Comparison

| Feature | v1 | v2 | v3 | v4 | v5 |
|---------|----|----|----|----|----|
| Root Makefile | ✅ | ✅ | ✅ | ✅ | ✅ |
| Auto-discover sources | ❌ | ❌ | ✅ | ✅ | ✅ |
| Interfaces include path | ❌ | ✅ | ✅ | ✅ | ✅ |
| FreeRTOS kernel includes | ❌ | ❌ | ❌ | ❌ | ✅ |
| Host test target | ❌ | ❌ | ❌ | ✅ | ✅ |
| Binary/Hex/Map output | ❌ | ❌ | ❌ | ❌ | ✅ |
| Dependency files (.d) | ✅ | ✅ | ✅ | ✅ | ✅ |

---

## Complete Root Makefile (v5)

```makefile
# ============================================================================
# Lattice FC - Complete Root Makefile
# ============================================================================
VENDOR := freertos-thejas32
VENDOR_BUILD := $(VENDOR)/build
VENDOR_DEMO := $(VENDOR)/Demo/RISCV_CDAC_THEJAS32
VENDOR_BSP_INC := $(VENDOR_DEMO)/thejas32_hal/drivers/vega-sdk_freertos_thejas32/bsp/include
FREERTOS_INC := $(VENDOR)/FreeRTOS_Source/include
FREERTOS_PORT := $(VENDOR)/FreeRTOS_Source/portable/GCC/RISC-V

TOOLCHAIN ?= $(HOME)/vega-tools/bin
PREFIX    ?= riscv32-vega-elf
CC        := $(TOOLCHAIN)/$(PREFIX)-gcc
OBJCOPY   := $(TOOLCHAIN)/$(PREFIX)-objcopy
OBJDUMP   := $(TOOLCHAIN)/$(PREFIX)-objdump
SIZE      := $(TOOLCHAIN)/$(PREFIX)-size

ARCH  := -march=rv32im_zicsr -mabi=ilp32 -mcmodel=medany
WARN  := -Wall -Wextra -Wshadow -Wpointer-arith
CFLAGS  := $(ARCH) $(WARN) -Og -g -std=gnu11 -ffunction-sections -fdata-sections -MMD -MP
LDFLAGS := $(ARCH) -T$(VENDOR_DEMO)/riscv-thejas32.ld -nostartfiles -Wl,--gc-sections -Wl,--print-memory-usage
LDLIBS  := -Wl,--start-group -lgcc -lc -lm -Wl,--end-group

YOUR_DIRS := app core devices interfaces config
YOUR_SRCS := $(foreach d,$(YOUR_DIRS),$(wildcard $(d)/**/*.c) $(wildcard $(d)/*.c))
YOUR_OBJS := $(addprefix build/,$(YOUR_SRCS:.c=.o))
VENDOR_OBJS := $(wildcard $(VENDOR_BUILD)/*.o)

INCLUDES := -Iinterfaces -Icore -Idevices -Iconfig -Iapp \
            -I$(FREERTOS_INC) -I$(FREERTOS_PORT) \
            -I$(VENDOR_BSP_INC) -I$(VENDOR_DEMO)

ELF := build/firmware.elf
BIN := build/firmware.bin
HEX := build/firmware.hex
MAP := build/firmware.map

.PHONY: all clean flash-sram flash-spi monitor size test-host help

all: $(ELF) $(BIN) $(HEX)
	@echo "✅ Build: $(ELF)"

$(ELF): $(YOUR_OBJS) $(VENDOR_OBJS) $(VENDOR_DEMO)/riscv-thejas32.ld
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(YOUR_OBJS) $(VENDOR_OBJS) $(LDLIBS) -o $@ -Wl,-Map=$(MAP)
	$(SIZE) $@
	$(OBJDUMP) -d $@ > $(ELF:.elf=.dump)

build/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BIN): $(ELF); $(OBJCOPY) -I elf32-littleriscv -O binary $< $@
$(HEX): $(ELF); $(OBJCOPY) -I elf32-littleriscv -O ihex $< $@

flash-sram: $(ELF)
	/home/siesta/flasher/vega-flasher/vegaxmodem /dev/ttyUSB0 $<

flash-spi: $(BIN)
	/home/siesta/flasher/vega-flasher/flasher.bat $<

monitor:; picocom --baud 115200 --flow n --echo /dev/ttyUSB0

size: $(ELF); $(SIZE) -A $<

clean:; rm -rf build

help:
	@echo "Targets: all, flash-sram, flash-spi, monitor, size, test-host, clean"

HOST_CC := gcc
HOST_CFLAGS := -Wall -Wextra -Og -g -Icore -Iinterfaces -Idevices
TEST_SRCS := $(wildcard tests/unit/*.c)
TEST_OBJS := $(addprefix build/host/,$(TEST_SRCS:.c=.o))

test-host: build/host/test_ekf build/host/test_pid
	@./build/host/test_ekf && ./build/host/test_pid

build/host/test_ekf: build/host/tests/unit/test_ekf.o build/host/core/estimation/ekf.o
	$(HOST_CC) $^ -o $@ -lm

build/host/%.o: %.c
	@mkdir -p $(@D)
	$(HOST_CC) $(HOST_CFLAGS) -c $< -o $@

-include $(YOUR_OBJS:.o=.d)
```

---

## Daily Workflow

```bash
# 1. Pull latest submodule
git submodule update --init --recursive

# 2. Build
make clean all

# 3. Edit code
vim devices/sensors/imu/icm42688.c

# 4. Host test (core algorithms)
make test-host

# 5. Flash & monitor
make flash-sram
make monitor
```

## Navigation

- [[Developer Notes]] — Project overview and phases
- [[Architecture Details]] — Detailed layer specifications
- [[Flash Workflow]] — Flashing to SRAM vs SPI FLASH
- [[API Reference]] — All available headers and functions

All notes link back to [[_index.md]].
