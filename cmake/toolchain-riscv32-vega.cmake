# ============================================================================
# CMake Toolchain for RISC-V 32-bit VEGA (THEJAS32/ARIES)
# ============================================================================
# Usage: cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-riscv32-vega.cmake
# ============================================================================

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv32)

# Toolchain location - override with -DVEGA_TOOLS=... or -DTOOLCHAIN_BIN=...
set(VEGA_TOOLS "$ENV{HOME}/vega-tools" CACHE PATH "Path to vega-tools install")
set(TOOLCHAIN_BIN "${VEGA_TOOLS}/bin" CACHE PATH "Path to toolchain bin directory")
set(CROSS_COMPILE "riscv32-vega-elf" CACHE STRING "Toolchain prefix")

# Compiler
set(CMAKE_C_COMPILER "${TOOLCHAIN_BIN}/${CROSS_COMPILE}-gcc")
set(CMAKE_ASM_COMPILER "${CMAKE_C_COMPILER}")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_BIN}/${CROSS_COMPILE}-g++")

# Tools
set(CMAKE_OBJCOPY "${TOOLCHAIN_BIN}/${CROSS_COMPILE}-objcopy")
set(CMAKE_OBJDUMP "${TOOLCHAIN_BIN}/${CROSS_COMPILE}-objdump")
set(CMAKE_SIZE "${TOOLCHAIN_BIN}/${CROSS_COMPILE}-size")
set(CMAKE_STRIP "${TOOLCHAIN_BIN}/${CROSS_COMPILE}-strip")
set(CMAKE_AR "${TOOLCHAIN_BIN}/${CROSS_COMPILE}-ar")
set(CMAKE_RANLIB "${TOOLCHAIN_BIN}/${CROSS_COMPILE}-ranlib")

# Architecture flags for THEJAS32 (RV32IM + Zicsr)
set(ARCH_FLAGS "-march=rv32im_zicsr -mabi=ilp32 -mcmodel=medany")

set(CMAKE_C_FLAGS_INIT "${ARCH_FLAGS} -ffunction-sections -fdata-sections -fno-builtin-printf -fno-builtin-puts -fno-builtin-memcmp -fno-builtin-memcpy -Wall -std=gnu11")
set(CMAKE_ASM_FLAGS_INIT "${ARCH_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${ARCH_FLAGS} -nostartfiles -Wl,--gc-sections -Wl,--print-memory-usage")

# Find library search paths
set(CMAKE_FIND_ROOT_PATH "${TOOLCHAIN_BIN}/../lib/gcc/${CROSS_COMPILE}/13.2.0" "${TOOLCHAIN_BIN}/../${CROSS_COMPILE}/lib")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Disable trying to run target binaries during cross-compilation
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

