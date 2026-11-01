# Core autopilot sources.
#
# The scaffold uses globs to keep the tree easy to extend. For a production
# build, replace them with explicit source lists so a new file cannot silently
# enter the flight image.

add_subdirectory(msg)

file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/drivers/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/hal/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/middleware/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/systemcmds/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../os/freertrtos/src/*.c")

file(GLOB_RECURSE BOARD_SOURCES CONFIGURE_DEPENDS
    "${FREERTRTOS_BOARD_DIR}/*.c"
    "${FREERTRTOS_BOARD_DIR}/hal/*.c")

if(EXISTS "${FREERTRTOS_BOARD_DIR}/startup.S")
    list(APPEND BOARD_SOURCES "${FREERTRTOS_BOARD_DIR}/startup.S")
endif()

add_library(freertrtos_core STATIC ${CORE_SOURCES} ${BOARD_SOURCES})

add_dependencies(freertrtos_core msg_gen)

target_include_directories(freertrtos_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/msg/generated"
    "${CMAKE_CURRENT_SOURCE_DIR}/hal/include"
    "${CMAKE_SOURCE_DIR}/os/freertrtos/include"
    "${FREERTRTOS_BOARD_DIR}"
    "${FREERTRTOS_BOARD_DIR}/hal")

target_link_libraries(freertrtos_core PUBLIC msg_headers)

target_compile_options(freertrtos_core PRIVATE -Wall -Wextra)

target_compile_definitions(freertrtos_core PUBLIC
    FREERTRTOS=1
    FREERTRTOS_BOARD_${FREERTRTOS_BOARD}=1)
