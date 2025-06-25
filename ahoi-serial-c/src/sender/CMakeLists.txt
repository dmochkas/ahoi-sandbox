
add_library(${SENDER_LIB_NAME} OBJECT "sender.c")
target_compile_definitions(${SENDER_LIB_NAME} PRIVATE SENDER_SERIAL_PORT="${SENDER_SERIAL_PORT}")

target_include_directories(${SENDER_LIB_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/include")

target_link_libraries(${SENDER_LIB_NAME} PUBLIC ${ASCON_LIB_NAME})