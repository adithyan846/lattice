add_executable(freertrtos_host_tests
    test_main.c
    test_math.c
    test_pid.c
    test_geo.c
    test_mission.c
    test_atmosphere.c
    test_rtl.c
    test_control.c
    test_log.c
    test_mavlink.c
    test_nav.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lib/math/vec3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lib/math/quat.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lib/math/pid.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lib/geo/geo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lib/mission/mission.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lib/atmosphere/atmosphere.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lib/rtl/rtl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lib/crc/crc16.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lib/crc/crc32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/mc_att_control/attitude_controller.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/mc_att_control/rate_controller.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/mc_pos_control/position_controller.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/control_allocator/mixer_quad_x.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/control_allocator/actuator_limits.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/logger/log_format.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/mavlink/mavlink_parser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/mavlink/mavlink_encoder.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/mavlink/mavlink_messages.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/mavlink/mavlink_mission.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/mavlink/mavlink_commands.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/navigator/waypoint.c
)

target_include_directories(freertrtos_host_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/msg/generated)

target_compile_options(freertrtos_host_tests PRIVATE -Wall -Wextra -Wpedantic)

if(NOT MSVC)
    target_link_libraries(freertrtos_host_tests PRIVATE m)
endif()

add_test(NAME freertrtos_host_unit COMMAND freertrtos_host_tests)