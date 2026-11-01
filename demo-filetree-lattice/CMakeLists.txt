cmake_minimum_required(VERSION 3.20)

project(freertrtos_autopilot
    VERSION 0.1.0
    DESCRIPTION "FreeRTOS-based multicopter autopilot scaffold"
    LANGUAGES C ASM)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(cmake/options.cmake)

if(FREERTRTOS_BUILD_FIRMWARE)
    include(cmake/freertrtos.cmake)
endif()

if(FREERTRTOS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests/host)
endif()

message(STATUS "freertrtos board ......... ${FREERTRTOS_BOARD}")
message(STATUS "freertrtos build firmware  ${FREERTRTOS_BUILD_FIRMWARE}")
message(STATUS "freertrtos build tests ... ${FREERTRTOS_BUILD_TESTS}")
