# RISC-V toolchain for the reference THEJAS32 target.
#
# Adjust C_COMPILER / ARCH to match the toolchain installed on your machine.
# The project has been written for a 32-bit RISC-V core with an FPU optional;
# soft-float is the safe default for bring-up.

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv32)

set(TOOLCHAIN_PREFIX "riscv32-unknown-elf-" CACHE STRING "RISC-V toolchain prefix")
set(CMAKE_C_COMPILER   "${TOOLCHAIN_PREFIX}gcc")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_PREFIX}gcc")
set(CMAKE_AR           "${TOOLCHAIN_PREFIX}ar")
set(CMAKE_OBJCOPY      "${TOOLCHAIN_PREFIX}objcopy")
set(CMAKE_SIZE         "${TOOLCHAIN_PREFIX}size")

set(FREERTRTOS_ARCH "rv32imac" CACHE STRING "RISC-V ISA string")
set(FREERTRTOS_ABI  "ilp32"    CACHE STRING "RISC-V ABI")

set(CMAKE_C_FLAGS_INIT
    "-march=${FREERTRTOS_ARCH} -mabi=${FREERTRTOS_ABI} -mcmodel=medany "
    "-ffunction-sections -fdata-sections -fno-common -Wall -Wextra")
set(CMAKE_ASM_FLAGS_INIT
    "-march=${FREERTRTOS_ARCH} -mabi=${FREERTRTOS_ABI} -mcmodel=medany")

set(CMAKE_EXE_LINKER_FLAGS_INIT
    "-nostartfiles -Wl,--gc-sections -Wl,--print-memory-usage")

# CMake cannot run target binaries during configure.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
