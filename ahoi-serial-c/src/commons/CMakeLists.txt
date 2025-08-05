add_library(${COMMONS_LIB_NAME} OBJECT
    "serial_utils.c"
    "cli_helper.c"
    "ahoi_service.c")

target_include_directories(${COMMONS_LIB_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include")

target_link_libraries(${COMMONS_LIB_NAME} PUBLIC ${ASCON_LIB_NAME})