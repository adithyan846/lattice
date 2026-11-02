# ============================================================================
# Lattice - CMake build for FreeRTOS on THEJAS32 (ARIES)
# ============================================================================
# This provides IDE support (VS Code, CLion, etc.) and an alternative build system.
#
# Usage:
#   cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-riscv32-vega.cmake
#   cmake --build build
# ============================================================================

cmake_minimum_required(VERSION 3.20)
project(lattice LANGUAGES C ASM)

# ---------------------------------------------------------------------------
# Toolchain configuration
# ---------------------------------------------------------------------------
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv32)

# Toolchain paths (can be overridden via -D)
set(VEGA_TOOLS "$ENV{HOME}/vega-tools" CACHE PATH "Path to vega-tools install")
set(TOOLCHAIN_BIN "${VEGA_TOOLS}/bin" CACHE PATH "Path to toolchain bin")
set(CROSS_COMPILE "riscv32-vega-elf" CACHE STRING "Toolchain prefix")

# Compiler
set(CMAKE_C_COMPILER "${TOOLCHAIN_BIN}/${CROSS_COMPILE}-gcc")
set(CMAKE_ASM_COMPILER "${CMAKE_C_COMPILER}")
set(CMAKE_OBJCOPY "${TOOLCHAIN_BIN}/${CROSS_COMPILE}-objcopy")
set(CMAKE_OBJDUMP "${TOOLCHAIN_BIN}/${CROSS_COMPILE}-objdump")
set(CMAKE_SIZE "${TOOLCHAIN_BIN}/${CROSS_COMPILE}-size")

# Architecture flags
set(ARCH_FLAGS "-march=rv32im_zicsr -mabi=ilp32 -mcmodel=medany")
set(CMAKE_C_FLAGS "${ARCH_FLAGS} -ffunction-sections -fdata-sections -fno-builtin-printf -fno-builtin-puts -fno-builtin-memcmp -fno-builtin-memcpy -Wall -std=gnu11")
set(CMAKE_ASM_FLAGS "${ARCH_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${ARCH_FLAGS} -nostartfiles -Wl,--gc-sections -Wl,--print-memory-usage")

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

# Debug/Release flags
set(CMAKE_C_FLAGS_DEBUG "-Og -g")
set(CMAKE_C_FLAGS_RELEASE "-O2 -g")

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
set(FREERTOS_DIR "${CMAKE_SOURCE_DIR}/freertos-thejas32")
set(DEMO_DIR "${FREERTOS_DIR}/Demo/RISCV_CDAC_THEJAS32")
set(SDK_DIR "${DEMO_DIR}/thejas32_hal/drivers/vega-sdk_freertos_thejas32")
set(BSP_DIR "${SDK_DIR}/bsp")
set(RISCV_HAL_DIR "${DEMO_DIR}/thejas32_hal/riscv_hal")
set(FREERTOS_SRC_DIR "${FREERTOS_DIR}/FreeRTOS_Source")
set(CHIP_EXT_DIR "${FREERTOS_SRC_DIR}/portable/GCC/RISC-V/chip_specific_extensions/VEGA-THEJA32-ARIES")

# ---------------------------------------------------------------------------
# Include directories
# ---------------------------------------------------------------------------
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${DEMO_DIR}
    ${FREERTOS_DIR}/Demo/Common/include
    ${FREERTOS_SRC_DIR}/include
    ${FREERTOS_SRC_DIR}/portable/GCC/RISC-V
    ${CHIP_EXT_DIR}
    ${BSP_DIR}
    ${BSP_DIR}/include
    ${RISCV_HAL_DIR}
)

# ---------------------------------------------------------------------------
# Definitions
# ---------------------------------------------------------------------------
add_compile_definitions(NOT_MSCC_STDIO_THRU_CORE_UART_APB)

# ---------------------------------------------------------------------------
# FreeRTOS Demo target
# ---------------------------------------------------------------------------
set(FREERTOS_SOURCES
    ${DEMO_DIR}/main.c
    ${RISCV_HAL_DIR}/init.c
    ${BSP_DIR}/common/stdlib.c
    ${BSP_DIR}/common/rawfloat.c
    ${BSP_DIR}/drivers/at25sf161_flash/at25sf161.c
    ${BSP_DIR}/drivers/gpio/gpio.c
    ${BSP_DIR}/drivers/gpio/led.c
    ${BSP_DIR}/drivers/i2c/i2c.c
    ${BSP_DIR}/drivers/interrupt/interrupt.c
    ${BSP_DIR}/drivers/pwm/pwmc.c
    ${BSP_DIR}/drivers/qspi/qspi.c
    ${BSP_DIR}/drivers/spi/spi.c
    ${BSP_DIR}/drivers/timer/timer.c
    ${BSP_DIR}/drivers/uart/debug_uart.c
    ${BSP_DIR}/drivers/uart/uart.c
    ${FREERTOS_SRC_DIR}/tasks.c
    ${FREERTOS_SRC_DIR}/list.c
    ${FREERTOS_SRC_DIR}/queue.c
    ${FREERTOS_SRC_DIR}/timers.c
    ${FREERTOS_SRC_DIR}/event_groups.c
    ${FREERTOS_SRC_DIR}/stream_buffer.c
    ${FREERTOS_SRC_DIR}/croutine.c
    ${FREERTOS_SRC_DIR}/portable/MemMang/heap_4.c
    ${FREERTOS_SRC_DIR}/portable/GCC/RISC-V/port.c
)

set(FREERTOS_ASM_SOURCES
    ${RISCV_HAL_DIR}/entry.S
    ${FREERTOS_SRC_DIR}/portable/GCC/RISC-V/portASM.S
)

add_executable(RTOSDemo ${FREERTOS_SOURCES} ${FREERTOS_ASM_SOURCES})

# Linker script
set_target_properties(RTOSDemo PROPERTIES
    LINK_FLAGS "-T${DEMO_DIR}/riscv-thejas32.ld"
)

# Post-build: generate bin/hex/lst/dump
add_custom_command(TARGET RTOSDemo POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -I elf32-littleriscv -O binary $<TARGET_FILE:RTOSDemo> $<TARGET_FILE_DIR:RTOSDemo>/RTOSDemo.bin
    COMMAND ${CMAKE_OBJCOPY} -I elf32-littleriscv -O ihex $<TARGET_FILE:RTOSDemo> $<TARGET_FILE_DIR:RTOSDemo>/RTOSDemo.hex
    COMMAND ${CMAKE_OBJDUMP} -d $<TARGET_FILE:RTOSDemo> > $<TARGET_FILE_DIR:RTOSDemo>/RTOSDemo.dump
    COMMAND ${CMAKE_OBJDUMP} --source --all-headers --demangle --line-numbers --wide $<TARGET_FILE:RTOSDemo> > $<TARGET_FILE_DIR:RTOSDemo>/RTOSDemo.lst
    COMMAND ${CMAKE_SIZE} -A $<TARGET_FILE:RTOSDemo>
    COMMENT "Generating binary, hex, and listings"
)

# ---------------------------------------------------------------------------
# User Application target (optional)
# ---------------------------------------------------------------------------
if(EXISTS "${CMAKE_SOURCE_DIR}/app/src/main.c")
    file(GLOB APP_SOURCES "${CMAKE_SOURCE_DIR}/app/src/*.c")
    
    add_executable(lattice_app ${APP_SOURCES})
    
    target_include_directories(lattice_app PRIVATE
        ${CMAKE_SOURCE_DIR}/app/src
    )
    
    # Link against FreeRTOS objects
    target_link_libraries(lattice_app RTOSDemo)
    
    set_target_properties(lattice_app PROPERTIES
        LINK_FLAGS "-T${DEMO_DIR}/riscv-thejas32.ld"
    )
    
    add_custom_command(TARGET lattice_app POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -I elf32-littleriscv -O binary $<TARGET_FILE:lattice_app> $<TARGET_FILE_DIR:lattice_app>/lattice_app.bin
        COMMAND ${CMAKE_OBJCOPY} -I elf32-littleriscv -O ihex $<TARGET_FILE:lattice_app> $<TARGET_FILE_DIR:lattice_app>/lattice_app.hex
        COMMENT "Generating app binary and hex"
    )
endif()

# ---------------------------------------------------------------------------
# Flash/Monitor targets (custom targets)
# ---------------------------------------------------------------------------
set(SERIAL_PORT "/dev/ttyUSB0" CACHE STRING "Serial port for flashing")
set(FLASH_TOOL "$ENV{HOME}/Desktop/vega-flasher/xmodemflasher" CACHE FILEPATH "Path to xmodemflasher")

add_custom_target(flash
    COMMAND ${FLASH_TOOL} ${SERIAL_PORT} $<TARGET_FILE_DIR:RTOSDemo>/RTOSDemo.bin
    DEPENDS RTOSDemo
    COMMENT "Flashing RTOSDemo to board"
)

add_custom_target(monitor
    COMMAND picocom --baud 115200 --flow n --echo ${SERIAL_PORT}
    DEPENDS flash
    COMMENT "Opening serial monitor"
)

# ---------------------------------------------------------------------------
# Install (copy artifacts to build root)
# ---------------------------------------------------------------------------
install(TARGETS RTOSDemo RUNTIME DESTINATION .)

