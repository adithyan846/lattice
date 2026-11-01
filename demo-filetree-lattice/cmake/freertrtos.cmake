# Firmware build wiring.
#
# This file is only included when FREERTRTOS_BUILD_FIRMWARE=ON. It expects:
#   * FreeRTOS-Kernel checkout          (FREERTRTOS_KERNEL_DIR)
#   * THEJAS32 FreeRTOS port checkout   (FREERTRTOS_PORT_DIR)
#
# The actual kernel source list and port file names differ slightly between
# FreeRTOS releases. Adjust the lists below if your checkout uses different
# file names.

if(NOT EXISTS "${FREERTRTOS_KERNEL_DIR}/tasks.c")
    message(FATAL_ERROR
        "FreeRTOS kernel not found at ${FREERTRTOS_KERNEL_DIR}.\n"
        "Clone https://github.com/FreeRTOS/FreeRTOS-Kernel there, or pass "
        "-DFREERTRTOS_KERNEL_DIR=/path/to/FreeRTOS-Kernel.")
endif()

if(NOT EXISTS "${FREERTRTOS_PORT_DIR}")
    message(FATAL_ERROR
        "FreeRTOS port not found at ${FREERTRTOS_PORT_DIR}.\n"
        "Clone the THEJAS32 port there, or pass "
        "-DFREERTRTOS_PORT_DIR=/path/to/freertos-thejas32.")
endif()

set(FREERTRTOS_KERNEL_SOURCES
    "${FREERTRTOS_KERNEL_DIR}/tasks.c"
    "${FREERTRTOS_KERNEL_DIR}/queue.c"
    "${FREERTRTOS_KERNEL_DIR}/list.c"
    "${FREERTRTOS_KERNEL_DIR}/timers.c"
    "${FREERTRTOS_KERNEL_DIR}/event_groups.c"
    "${FREERTRTOS_KERNEL_DIR}/stream_buffer.c"
    "${FREERTRTOS_KERNEL_DIR}/portable/MemMang/heap_4.c")

# The port checkout is board/ISA specific; pick the RISC-V port when present.
if(EXISTS "${FREERTRTOS_PORT_DIR}/port.c")
    list(APPEND FREERTRTOS_KERNEL_SOURCES "${FREERTRTOS_PORT_DIR}/port.c")
elseif(EXISTS "${FREERTRTOS_PORT_DIR}/portable/GCC/RISC-V/port.c")
    list(APPEND FREERTRTOS_KERNEL_SOURCES
        "${FREERTRTOS_PORT_DIR}/portable/GCC/RISC-V/port.c")
    list(APPEND FREERTRTOS_KERNEL_SOURCES
        "${FREERTRTOS_PORT_DIR}/portable/GCC/RISC-V/portASM.S")
else()
    message(FATAL_ERROR
        "Could not find port.c in ${FREERTRTOS_PORT_DIR}. "
        "Set FREERTRTOS_PORT_DIR to the directory containing the THEJAS32 port.")
endif()

add_library(freertos_kernel STATIC ${FREERTRTOS_KERNEL_SOURCES})

target_include_directories(freertos_kernel PUBLIC
    "${FREERTRTOS_KERNEL_DIR}/include"
    "${FREERTRTOS_PORT_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/os/freertrtos/port/thejas32"
    "${CMAKE_CURRENT_SOURCE_DIR}/os/freertrtos/include")

target_compile_definitions(freertos_kernel PUBLIC
    FREERTRTOS=1
    FREERTRTOS_BOARD_${FREERTRTOS_BOARD}=1)

if(EXISTS "${FREERTRTOS_MAVLINK_DIR}/CMakeLists.txt")
    add_subdirectory("${FREERTRTOS_MAVLINK_DIR}" mavlink_build)
endif()

add_subdirectory(src)

add_executable(freertrtos_autopilot
    src/main/main.c
    src/main/init.c
    src/main/task_registry.c
    src/main/version.c)
target_link_libraries(freertrtos_autopilot PRIVATE freertrtos_core freertos_kernel)
target_include_directories(freertrtos_autopilot PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/msg/generated")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/boards/${FREERTRTOS_BOARD}/ld/link.ld")
    target_link_options(freertrtos_autopilot PRIVATE
        "-T${CMAKE_CURRENT_SOURCE_DIR}/boards/${FREERTRTOS_BOARD}/ld/link.ld"
        "-Wl,--gc-sections"
        "-Wl,-Map=${CMAKE_BINARY_DIR}/freertrtos_autopilot.map")
endif()
