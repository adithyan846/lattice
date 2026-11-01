# Freertrtos Autopilot build options.
#
# The firmware build is intentionally opt-in: host unit tests must work on a
# developer laptop without a RISC-V toolchain or a FreeRTOS checkout.

set(FREERTRTOS_BOARD "thejas32" CACHE STRING "Target board: thejas32 or sitl")

option(FREERTRTOS_BUILD_FIRMWARE "Build the target firmware" ON)
option(FREERTRTOS_BUILD_TESTS "Build host unit tests" ON)
option(FREERTRTOS_WERROR "Treat warnings as errors" OFF)

set(FREERTRTOS_KERNEL_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS-Kernel"
    CACHE PATH "Path to the upstream FreeRTOS-Kernel checkout")

set(FREERTRTOS_PORT_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/freertos-thejas32"
    CACHE PATH "Path to the THEJAS32 FreeRTOS port checkout")

set(FREERTRTOS_MAVLINK_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/mavlink"
    CACHE PATH "Path to the MAVLink C library checkout")

set(FREERTRTOS_BOARD_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/boards/${FREERTRTOS_BOARD}")

if(NOT EXISTS "${FREERTRTOS_BOARD_DIR}")
    message(FATAL_ERROR
        "Unknown board '${FREERTRTOS_BOARD}'. Expected ${FREERTRTOS_BOARD_DIR}.")
endif()

if(FREERTRTOS_WERROR)
    add_compile_options(-Werror)
endif()
