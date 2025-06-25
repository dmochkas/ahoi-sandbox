add_library(${COMMONS_LIB_NAME} OBJECT
    "serial_utils.c")

target_include_directories(${COMMONS_LIB_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include")