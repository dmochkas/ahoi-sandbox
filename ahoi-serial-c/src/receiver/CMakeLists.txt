
add_library(${RECEIVER_LIB_NAME} OBJECT "receiver.c")
target_compile_definitions(${RECEIVER_LIB_NAME} PRIVATE RECEIVER_SERIAL_PORT="${RECEIVER_SERIAL_PORT}")

target_include_directories(${RECEIVER_LIB_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/include")

target_link_libraries(${RECEIVER_LIB_NAME} PUBLIC ${ASCON_LIB_NAME})