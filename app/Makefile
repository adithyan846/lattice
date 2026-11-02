# ============================================================================
# Lattice Application Makefile
# Build user applications on top of FreeRTOS/THEJAS32
# ============================================================================

APP_NAME        ?= lattice_app
APP_SRC_DIR     ?= src
APP_BUILD_DIR   ?= build

# FreeRTOS location (relative to this Makefile)
FREERTOS_DIR    ?= ../freertos-thejas32

# Toolchain (passed from top-level or environment)
TOOLCHAIN_BIN   ?= $(HOME)/vega-tools/bin
CROSS_COMPILE   ?= riscv32-vega-elf

CC       := $(TOOLCHAIN_BIN)/$(CROSS_COMPILE)-gcc
OBJCOPY  := $(TOOLCHAIN_BIN)/$(CROSS_COMPILE)-objcopy
OBJDUMP  := $(TOOLCHAIN_BIN)/$(CROSS_COMPILE)-objdump
SIZE     := $(TOOLCHAIN_BIN)/$(CROSS_COMPILE)-size

# Architecture (THEJAS32 = RV32IM + Zicsr)
ARCH    := -march=rv32im_zicsr
ABI     := -mabi=ilp32
CMODEL  := -mcmodel=medany
XLEN    := 32

# Source files
APP_SOURCES := $(wildcard $(APP_SRC_DIR)/*.c)
APP_ASM_SOURCES := $(wildcard $(APP_SRC_DIR)/*.S)
APP_OBJECTS := $(addprefix $(APP_BUILD_DIR)/,$(APP_SOURCES:.c=.o)) \
               $(addprefix $(APP_BUILD_DIR)/,$(APP_ASM_SOURCES:.S=.o))

# Include paths - FreeRTOS + HAL + BSP
FREERTOS_INC := -I$(FREERTOS_DIR)/FreeRTOS_Source/include \
                -I$(FREERTOS_DIR)/FreeRTOS_Source/portable/GCC/RISC-V \
                -I$(FREERTOS_DIR)/FreeRTOS_Source/portable/GCC/RISC-V/chip_specific_extensions/VEGA-THEJA32-ARIES

HAL_INC := -I$(FREERTOS_DIR)/Demo/RISCV_CDAC_THEJAS32/thejas32_hal/riscv_hal \
           -I$(FREERTOS_DIR)/Demo/RISCV_CDAC_THEJAS32/thejas32_hal/drivers/vega-sdk_freertos_thejas32/bsp/include \
           -I$(FREERTOS_DIR)/Demo/RISCV_CDAC_THEJAS32/thejas32_hal/drivers/vega-sdk_freertos_thejas32/bsp

DEMO_INC := -I$(FREERTOS_DIR)/Demo/RISCV_CDAC_THEJAS32 \
            -I$(FREERTOS_DIR)/Demo/Common/include

INCLUDES := -I$(APP_SRC_DIR) $(FREERTOS_INC) $(HAL_INC) $(DEMO_INC)

DEFINES  := -DNOT_MSCC_STDIO_THRU_CORE_UART_APB

COMMON_FLAGS := $(ARCH) $(ABI) $(CMODEL) $(DEFINES) \
                -ffunction-sections -fdata-sections \
                -fno-builtin-printf -fno-builtin-puts -fno-builtin-memcmp -fno-builtin-memcpy \
                -Wall

OPT ?= -Og -g

CFLAGS   := $(COMMON_FLAGS) $(INCLUDES) $(OPT) -std=gnu11 -MMD -MP
ASMFLAGS := $(COMMON_FLAGS) $(INCLUDES) $(OPT) -MMD -MP

LDSCRIPT := $(FREERTOS_DIR)/Demo/RISCV_CDAC_THEJAS32/riscv-thejas32.ld

LDFLAGS := $(ARCH) $(ABI) $(CMODEL) \
           -T$(LDSCRIPT) \
           -nostartfiles \
           -Wl,--gc-sections \
           -Wl,-Map=$(APP_BUILD_DIR)/$(APP_NAME).map \
           -Wl,--print-memory-usage

LDLIBS := -Wl,--start-group -lgcc -lc -lm -Wl,--end-group

# FreeRTOS pre-built objects to link against
FREERTOS_BUILD_DIR := $(FREERTOS_DIR)/build
FREERTOS_OBJS := $(wildcard $(FREERTOS_BUILD_DIR)/*.o)

# If FreeRTOS not built yet, we can build it
FREERTOS_MAKEFILE := $(FREERTOS_DIR)/Makefile

.PHONY: all clean freertos sample

all: $(APP_BUILD_DIR)/$(APP_NAME).elf

# Build FreeRTOS first if needed
$(FREERTOS_OBJS): $(FREERTOS_MAKEFILE)
	@echo "FreeRTOS not built, building..."
	@$(MAKE) -C $(FREERTOS_DIR) TOOLCHAIN_BIN=$(TOOLCHAIN_BIN) CROSS_COMPILE=$(CROSS_COMPILE) build

$(APP_BUILD_DIR)/%.o: $(APP_SRC_DIR)/%.c
	@mkdir -p $(@D)
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(APP_BUILD_DIR)/%.o: $(APP_SRC_DIR)/%.S
	@mkdir -p $(@D)
	@echo "  AS      $<"
	@$(CC) $(ASMFLAGS) -c $< -o $@

$(APP_BUILD_DIR)/$(APP_NAME).elf: $(APP_OBJECTS) $(LDSCRIPT) $(FREERTOS_OBJS)
	@echo "  LD      $@"
	@$(CC) $(LDFLAGS) $(APP_OBJECTS) $(FREERTOS_OBJS) $(LDLIBS) -o $@
	@$(SIZE) $@
	@$(OBJDUMP) -d $@ > $(APP_BUILD_DIR)/$(APP_NAME).dump
	@$(OBJDUMP) --source --all-headers --demangle --line-numbers --wide $@ > $(APP_BUILD_DIR)/$(APP_NAME).lst

$(APP_BUILD_DIR)/$(APP_NAME).bin: $(APP_BUILD_DIR)/$(APP_NAME).elf
	@$(OBJCOPY) -I elf$(XLEN)-littleriscv -O binary $< $@

$(APP_BUILD_DIR)/$(APP_NAME).hex: $(APP_BUILD_DIR)/$(APP_NAME).elf
	@$(OBJCOPY) -I elf$(XLEN)-littleriscv -O ihex $< $@

clean:
	@rm -rf $(APP_BUILD_DIR)

# Create sample main.c if none exists
sample:
	@mkdir -p $(APP_SRC_DIR)
	@if [ ! -f $(APP_SRC_DIR)/main.c ]; then \
	    cat > $(APP_SRC_DIR)/main.c << 'SAMPLEEOF'; \
#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include "uart.h"
#include "gpio.h"
#include "led.h"

static void app_task(void *pvParameters) {
    const char *msg = "Lattice App Running!\n\r";
    while (1) {
        vSendString(msg);
        vToggleLED();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main(void) {
    xTaskCreate(app_task, "AppTask", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, NULL);
    vTaskStartScheduler();
    for (;;);
}
SAMPLEEOF
	    echo "Created sample $(APP_SRC_DIR)/main.c"; \
	fi

# Include dependency files
-include $(APP_OBJECTS:.o=.d)

