# ============================================================================
#  FreeRTOS on CDAC VEGA "THEJAS32" (ARIES) - Makefile
#  Replaces the original Eclipse Managed Build (.cproject / .settings) flow.
#
#  Targets
#    make            -> build   (build/<name>.elf/.bin/.hex + map + listing)
#    make clean      -> remove all build artefacts
#    make flash      -> upload the .bin over the ARIES xmodem loader
#    make flash HEX  -> same as above (kept as an alias for convenience)
#    make run        -> build + flash
#    make monitor    -> flash + open the serial console (picocom, 115200)
#    make size       -> print size of the linked ELF
#    make disasm     -> regenerate the disassembly listing
#    make info       -> print the resolved toolchain / board configuration
#
#  Everything below is overridable from the command line, e.g.
#    make TOOLCHAIN_BIN=/opt/vega-tools-rv32/toolchain/bin
#    make SERIAL_PORT=/dev/ttyACM0 BAUD=921600
# ============================================================================

# ---------------------------------------------------------------------------
# Build mode selection
# ---------------------------------------------------------------------------
WITH_FREERTOS ?= 0

ifeq ($(WITH_FREERTOS),1)
    CFLAGS += -DWITH_FREERTOS
endif

# ---------------------------------------------------------------------------
# Board / toolchain selection
# ---------------------------------------------------------------------------
TARGET            ?= THEJAS32
CROSS_COMPILE     ?= riscv32-vega-elf

# Base install of vega-tools-rv32. Override if you moved it.
VEGA_TOOLS        ?= $(HOME)/vega-tools
# Either point directly at .../toolchain/bin, or let it be derived from the
# VEGA_TOOLS root above.
TOOLCHAIN_BIN     ?= $(VEGA_TOOLS)/bin
TOOLCHAIN_PREFIX  ?= $(VEGA_TOOLS)/

CC       := $(TOOLCHAIN_BIN)/$(CROSS_COMPILE)-gcc
OBJCOPY  := $(TOOLCHAIN_BIN)/$(CROSS_COMPILE)-objcopy
OBJDUMP  := $(TOOLCHAIN_BIN)/$(CROSS_COMPILE)-objdump
SIZE     := $(TOOLCHAIN_BIN)/$(CROSS_COMPILE)-size
GDB      := $(TOOLCHAIN_BIN)/$(CROSS_COMPILE)-gdb

# ---------------------------------------------------------------------------
# gcc runtime dependencies
#
# The vega gcc 13.x cc1 links against libisl.so.22, which is not shipped by
# current distributions (only libisl.so.23 is). Point this at a directory
# holding libisl.so.22 (e.g. a locally built isl-0.22 install) if the
# compiler dies with "error while loading shared libraries: libisl.so.22".
# Leave empty to use whatever the system loader provides.
# ---------------------------------------------------------------------------
TOOLCHAIN_LIB     ?= $(HOME)/vega-tools/lib

# ---------------------------------------------------------------------------
# Architecture / ABI  (per the vega-sdk bsp/common/config.mk: THEJAS32)
# ---------------------------------------------------------------------------
ifeq ($(TARGET),THEJAS32)
  ARCH    := -march=rv32im_zicsr
  ABI     := -mabi=ilp32
  XLEN    := 32
endif
CMODEL   ?= -mcmodel=medany

# ---------------------------------------------------------------------------
# Project layout
# ---------------------------------------------------------------------------
ROOT := $(CURDIR)
FREERTOS_DIR := $(ROOT)/freertos-thejas32
BUILD_DIR   ?= $(ROOT)/build
TARGET_NAME ?= firmware

DEMO        := $(FREERTOS_DIR)/Demo/RISCV_CDAC_THEJAS32
SDK         := $(DEMO)/thejas32_hal/drivers/vega-sdk_freertos_thejas32
BSP         := $(SDK)/bsp
RISCV_HAL   := $(DEMO)/thejas32_hal/riscv_hal
FREERTOS    := $(FREERTOS_DIR)/FreeRTOS_Source

CHIP_EXT    := $(FREERTOS)/portable/GCC/RISC-V/chip_specific_extensions/VEGA-THEJA32-ARIES

# Explicit Lattice sources (using wildcard function for subdirs)
LATTICE_SRCS := \
	app/src/main.c \
	$(wildcard core/*.c) \
	$(wildcard devices/*.c) \
	$(wildcard interfaces/*.c) \
	$(wildcard hal/*.c) \
	$(wildcard utils/*.c) \
	platform/thejas32/board/board.c \
	platform/thejas32/gpio.c \
	platform/thejas32/timer.c \
	platform/thejas32/uart.c

# Select SPI HAL based on build mode
ifeq ($(WITH_FREERTOS),1)
    LATTICE_SRCS += platform/thejas32/spi_freertos.c
else
    LATTICE_SRCS += platform/thejas32/spi_baremetal.c
endif

LATTICE_INCLUDES := \
	-I$(ROOT)/app/include \
	-I$(ROOT)/core/include \
	-I$(ROOT)/devices/bmp280 \
	-I$(ROOT)/interfaces/barometer \
	-I$(ROOT)/hal/include \
	-I$(ROOT)/utils/include \
	-I$(ROOT)/platform/thejas32 \
	-I$(ROOT)/platform/thejas32/board

# OLD: Unused rwildcard and LATTICE_DIRS approach (commented out)
# define rwildcard
# $(foreach d,$(wildcard $(1)/*), \
# 	$(call rwildcard,$(d),$(2))) \
# 	$(wildcard $(1)/$(2))
# endef
#
# LATTICE_DIRS := \
# 				app \
# 				core \
# 				devices \
# 				interfaces \
# 			    hal \
# 				utils \
# 				platform
#
# LATTICE_SRCS := $(foreach dir,$(LATTICE_DIRS), \
# 			$(call rwildcard,$(LATTICE)/$(dir),*.c))

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------
C_SOURCES := \
  $(LATTICE_SRCS) \
  $(RISCV_HAL)/init.c \
  $(BSP)/common/stdlib.c \
  $(BSP)/common/rawfloat.c \
  $(BSP)/drivers/at25sf161_flash/at25sf161.c \
  $(BSP)/drivers/gpio/gpio.c \
  $(BSP)/drivers/gpio/led.c \
  $(BSP)/drivers/i2c/i2c.c \
  $(BSP)/drivers/interrupt/interrupt.c \
  $(BSP)/drivers/pwm/pwmc.c \
  $(BSP)/drivers/qspi/qspi.c \
  $(BSP)/drivers/spi/spi.c \
  $(BSP)/drivers/timer/timer.c \
  $(BSP)/drivers/uart/debug_uart.c \
  $(BSP)/drivers/uart/uart.c \
  $(ROOT)/interfaces/barometer/barometer.c \
  $(ROOT)/devices/bmp280/bmp280.c \
  $(FREERTOS)/tasks.c \
  $(FREERTOS)/list.c \
  $(FREERTOS)/queue.c \
  $(FREERTOS)/timers.c \
  $(FREERTOS)/event_groups.c \
  $(FREERTOS)/stream_buffer.c \
  $(FREERTOS)/croutine.c \
  $(FREERTOS)/portable/MemMang/heap_4.c \
  $(FREERTOS)/portable/GCC/RISC-V/port.c

ASM_SOURCES := \
  $(RISCV_HAL)/entry.S \
  $(FREERTOS)/portable/GCC/RISC-V/portASM.S

# Object paths preserve the source tree layout under $(BUILD_DIR).
OBJECTS := $(addprefix $(BUILD_DIR)/,$(C_SOURCES:.c=.o)) \
           $(addprefix $(BUILD_DIR)/,$(ASM_SOURCES:.S=.o))

# ---------------------------------------------------------------------------
# Flags
# ---------------------------------------------------------------------------
# Include order matters: the BSP ships its own stdlib.h / printf which must
# shadow newlib, exactly as the Eclipse build had them ordered.
INCLUDES := \
  -I$(LATTICE_INCLUDES) \
  -I$(FREERTOS_DIR) \
  -I$(DEMO) \
  -I$(FREERTOS_DIR)/Demo/Common/include \
  -I$(FREERTOS)/include \
  -I$(FREERTOS)/portable/GCC/RISC-V \
  -I$(CHIP_EXT) \
  -I$(BSP) \
  -I$(BSP)/include \
  -I$(RISCV_HAL)

# NOT_MSCC_STDIO_THRU_CORE_UART_APB keeps hw_platform.h from demanding a
# CoreUART-apb STDIO mapping that this board does not have.
DEFINES := -DNOT_MSCC_STDIO_THRU_CORE_UART_APB

# -fno-builtin-* stops gcc from turning the BSP's own memset/memcpy/printf
# into self-referential calls.
COMMON_FLAGS := \
  $(ARCH) $(ABI) $(CMODEL) \
  $(DEFINES) \
  -ffunction-sections -fdata-sections \
  -fno-builtin-printf -fno-builtin-puts -fno-builtin-memcmp -fno-builtin-memcpy \
  -Wall

OPT ?= -Og -g

CFLAGS      := $(COMMON_FLAGS) $(INCLUDES) $(OPT) -std=gnu11 -MMD -MP
# The FreeRTOS port selects its chip-specific macros via the *assembler*
# include path, so ASM gets INCLUDES too (harmless, and keeps it simple).
ASMFLAGS    := $(COMMON_FLAGS) $(INCLUDES) $(OPT) -MMD -MP

LDSCRIPT := $(DEMO)/riscv-thejas32.ld

LDFLAGS := \
  $(ARCH) $(ABI) $(CMODEL) \
  -T$(LDSCRIPT) \
  -nostartfiles \
  -Wl,--gc-sections \
  -Wl,-Map=$(BUILD_DIR)/$(TARGET_NAME).map \
  -Wl,--print-memory-usage

# libgcc is needed for the compiler's soft helpers. We deliberately do not
# pass -nostdlib: newlib supplies crt0 glue we do not want alongside the
# custom entry.S, but -nostartfiles already keeps crt0.o out.
# printf/puts/memset/memcpy come from the BSP, which overrides newlib's
# archive members because they are defined in an object file directly.
LDLIBS := -Wl,--start-group -lgcc -lc -lm -Wl,--end-group

# ---------------------------------------------------------------------------
# Flash configuration (ARIES / THEJAS32 board)
# ---------------------------------------------------------------------------
SERIAL_PORT ?= /dev/ttyUSB0
FLASH_TOOL  ?= $(HOME)/flasher/vega-flasher/xmodemflasher
FLASH_BAUD  ?= 115200

# Serial console used by `make monitor`.
MONITOR_BAUD ?= 115200
PICOCOM      ?= picocom

# ---------------------------------------------------------------------------
# Artifacts
# ---------------------------------------------------------------------------
ELF := $(BUILD_DIR)/$(TARGET_NAME).elf
BIN := $(BUILD_DIR)/$(TARGET_NAME).bin
HEX := $(BUILD_DIR)/$(TARGET_NAME).hex
LST := $(BUILD_DIR)/$(TARGET_NAME).lst
DMP := $(BUILD_DIR)/$(TARGET_NAME).dump

# ---------------------------------------------------------------------------
# Toolchain environment (libisl.so.22 shim)
# ---------------------------------------------------------------------------
ifneq ($(strip $(TOOLCHAIN_LIB)),)
  export LD_LIBRARY_PATH := $(TOOLCHAIN_LIB)$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))
endif

# ---------------------------------------------------------------------------
# Rules
# ---------------------------------------------------------------------------
.PHONY: all build clean distclean flash run monitor size disasm info help check-toolchain check-serial
.PHONY: sources

sources:
	@printf '%s\n' $(LATTICE_SRCS)

all: build

# Fail early and with a useful message instead of a wall of make errors.
# Note: `$(CC) --version` is not enough to validate the toolchain - the driver
# works even when cc1 cannot load libisl.so.22. So we actually compile.
check-toolchain:
	@command -v $(CC) >/dev/null 2>&1 || { \
	    echo "error: compiler not found: $(CC)"; \
	    echo "       set TOOLCHAIN_BIN=/path/to/vega-tools-rv32/toolchain/bin"; \
	    exit 1; }
	@echo 'int main(void){return 0;}' | $(CC) $(ARCH) $(ABI) $(CMODEL) \
	    -x c -c - -o /dev/null 2>/tmp/.vega-tc-check.log || { \
	    echo "error: toolchain cannot compile for $(ARCH) $(ABI)."; \
	    sed 's/^/       /' /tmp/.vega-tc-check.log; \
	    if grep -q 'libisl' /tmp/.vega-tc-check.log; then \
	      echo "       vega gcc 13.x needs libisl.so.22 (distros ship only .23)."; \
	      echo "       point TOOLCHAIN_LIB at an isl-0.22 install, e.g."; \
	      echo "           make TOOLCHAIN_LIB=/opt/isl-0.22/install/lib"; \
	    fi; \
	    rm -f /tmp/.vega-tc-check.log; \
	    exit 1; }
	@rm -f /tmp/.vega-tc-check.log

build: check-toolchain $(BIN) $(HEX)
	@echo ""
	@echo "Built:"
	@echo "  ELF : $(ELF)"
	@echo "  BIN : $(BIN)"
	@echo "  HEX : $(HEX)"
	@echo "  MAP : $(BUILD_DIR)/$(TARGET_NAME).map"
	@echo ""

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(@D)
	@echo "  AS      $<"
	@$(CC) $(ASMFLAGS) -c $< -o $@

$(ELF): $(OBJECTS) $(LDSCRIPT)
	@echo "  LD      $@"
	@$(CC) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@
	@$(SIZE) $@
	@$(OBJDUMP) -d $@ > $(DMP)
	@$(OBJDUMP) --source --all-headers --demangle --line-numbers --wide $@ > $(LST)

# BIN/HEX used to be side effects of the ELF recipe, which left make with no
# rule to satisfy `flash: $(BIN)` on a clean tree. Give them real rules so
# `make flash` / `make monitor` depend on them properly.
$(BIN): $(ELF)
	@echo "  OBJCOPY $@"
	@$(OBJCOPY) -I elf$(XLEN)-littleriscv -O binary $< $@

$(HEX): $(ELF)
	@echo "  OBJCOPY $@"
	@$(OBJCOPY) -I elf$(XLEN)-littleriscv -O ihex   $< $@

size: $(ELF)
	@$(SIZE) -A $(ELF)

disasm: $(ELF)
	@$(OBJDUMP) -d $(ELF) > $(DMP)
	@$(OBJDUMP) --source --all-headers --demangle --line-numbers --wide $(ELF) > $(LST)
	@echo "  DMP : $(DMP)"
	@echo "  LST : $(LST)"

# Pre-flight for the serial port. The ARIES board enumerates as a CP2102 on
# /dev/ttyUSB0, which is root:dialout mode 0660, so a user who is not in the
# dialout group gets a bare "open: Permission denied" from the flash tool.
# Catch it here and print the actual fix instead.
check-serial:
	@test -e "$(SERIAL_PORT)" || { \
	    echo "error: serial port $(SERIAL_PORT) not found."; \
	    echo "       plug in the ARIES board and set SERIAL_PORT=..."; \
	    ls -1 /dev/ttyUSB* /dev/ttyACM* 2>/dev/null | sed 's/^/       found: /'; \
	    exit 1; }
	@test -r "$(SERIAL_PORT)" && test -w "$(SERIAL_PORT)" || { \
	    echo "error: no read/write access to $(SERIAL_PORT)"; \
	    ls -l "$(SERIAL_PORT)" | sed 's/^/       /'; \
	    echo "       your groups: $$(id -nG)"; \
	    if ! id -nG | tr ' ' '\n' | grep -qx dialout; then \
	        echo "       fix: sudo usermod -aG dialout $$(whoami)"; \
	        echo "            then log out and back in (group changes need a new login)."; \
	    fi; \
	    echo "       temporary workaround without sudo:"; \
	    echo "           sudo chmod a+rw $(SERIAL_PORT)"; \
	    exit 1; }

flash: $(BIN) check-serial
	@echo "  FLASH  $(BIN) -> $(SERIAL_PORT)"
	@if [ ! -x "$(FLASH_TOOL)" ]; then \
	    echo "error: flash tool not found/executable: $(FLASH_TOOL)"; \
	    echo "       set FLASH_TOOL=/path/to/xmodemflasher"; \
	    exit 1; \
	fi
	@echo "  Press the RESET button on the ARIES board now if it does not"
	@echo "  auto-enter the loader."
	@$(FLASH_TOOL) $(SERIAL_PORT) $(BIN)

run: flash

# Flash first, then hand the serial port over to picocom.
monitor: flash
	@command -v $(PICOCOM) >/dev/null 2>&1 || { \
	    echo "error: $(PICOCOM) not found in PATH."; \
	    echo "       install it (e.g. 'sudo apt install picocom') or set PICOCOM=..."; \
	    exit 1; }
	@echo "  MONITOR $(SERIAL_PORT) @ $(MONITOR_BAUD) baud (Ctrl-A Ctrl-Q to quit)"
	@$(PICOCOM) --baud $(MONITOR_BAUD) --flow n --echo $(SERIAL_PORT)

clean:
	@echo "  CLEAN  $(BUILD_DIR)"
	@rm -rf $(BUILD_DIR)

distclean: clean

info:
	@echo "Target        : $(TARGET)"
	@echo "Prefix        : $(CROSS_COMPILE)"
	@echo "Toolchain bin : $(TOOLCHAIN_BIN)"
	@echo "CC            : $(CC)"
	@echo "Arch / ABI    : $(ARCH) $(ABI) $(CMODEL)"
	@echo "Linker script : $(LDSCRIPT)"
	@echo "Serial port   : $(SERIAL_PORT)"
	@echo "Flash tool    : $(FLASH_TOOL)"
	@echo "Monitor       : $(PICOCOM) --baud $(MONITOR_BAUD) $(SERIAL_PORT)"
	@echo "Access        : $$([ -r "$(SERIAL_PORT)" ] && [ -w "$(SERIAL_PORT)" ] \
	    && echo ok || echo 'DENIED (need dialout group)')"
	@echo "C sources     : $(words $(C_SOURCES))"
	@echo "ASM sources   : $(words $(ASM_SOURCES))"

help:
	@echo "make            build the firmware (bare-metal SPI HAL)"
	@echo "make WITH_FREERTOS=1  build with FreeRTOS SPI HAL (mutex + timeout)"
	@echo "make clean      remove build artefacts"
	@echo "make flash      upload the .bin to the ARIES board over xmodem"
	@echo "make run        build + flash"
	@echo "make monitor    flash, then open the serial console in picocom"
	@echo "                (Ctrl-A Ctrl-Q quits picocom)"
	@echo "make size       show section sizes"
	@echo "make disasm     regenerate disassembly listings"
	@echo "make info       show resolved configuration"
	@echo "make check-serial  verify the serial port is present and accessible"

-include $(OBJECTS:.o=.d)
