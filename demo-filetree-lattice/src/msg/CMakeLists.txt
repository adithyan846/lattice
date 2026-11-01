set(MSG_INPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(MSG_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated")
set(MSG_OUTPUT "${MSG_OUTPUT_DIR}/uorb_topics.h")
set(MSG_FILES)

file(GLOB MSG_FILES CONFIGURE_DEPENDS "${MSG_INPUT_DIR}/*.msg")

add_custom_command(
    OUTPUT "${MSG_OUTPUT}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${MSG_OUTPUT_DIR}"
    COMMAND python3 "${CMAKE_SOURCE_DIR}/tools/msg_gen.py"
            --input "${MSG_INPUT_DIR}" --output "${MSG_OUTPUT_DIR}"
    DEPENDS ${MSG_FILES} "${CMAKE_SOURCE_DIR}/tools/msg_gen.py"
    COMMENT "Generating uORB topic structs"
    VERBATIM)

add_custom_target(msg_gen DEPENDS "${MSG_OUTPUT}")

add_library(msg_headers INTERFACE)
target_include_directories(msg_headers INTERFACE "${MSG_OUTPUT_DIR}")
