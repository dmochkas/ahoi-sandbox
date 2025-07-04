cmake_minimum_required(VERSION 3.10)
project(ahoi_serial C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(GitAddSubmodule)
include(config.cmake)
include(sender-config.cmake)
include(receiver-config.cmake)

set(ASCON_LIB_NAME "crypto_aead_${ALG_NAME}_${IMPL_LIST}")



set(COMMONS_LIB_NAME "commons-lib")
set(SENDER_TARGET "sender")
set(SENDER_LIB_NAME "sender-objects")
set(RECEIVER_TARGET "receiver")
set(RECEIVER_LIB_NAME "receiver-objects")

add_git_submodule(DIRECTORY thirdparty/ascon-c
        PROPAGATE ON
        PATCHES "patches/ascon-fix.patch")
add_subdirectory(src/commons)
add_subdirectory(src/sender)
add_subdirectory(src/receiver)


add_executable(${SENDER_TARGET} $<TARGET_OBJECTS:${SENDER_LIB_NAME}> $<TARGET_OBJECTS:${COMMONS_LIB_NAME}>)
add_executable(${RECEIVER_TARGET} $<TARGET_OBJECTS:${RECEIVER_LIB_NAME}> $<TARGET_OBJECTS:${COMMONS_LIB_NAME}>)

target_link_libraries(${SENDER_TARGET} PRIVATE ${ASCON_LIB_NAME})
target_link_libraries(${RECEIVER_TARGET} PRIVATE ${ASCON_LIB_NAME})

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13.0")
    target_link_options(${SENDER_TARGET} PRIVATE $<$<CONFIG:DEBUG>:-fsanitize=address,undefined>)
    target_link_options(${RECEIVER_TARGET} PRIVATE $<$<CONFIG:DEBUG>:-fsanitize=address,undefined>)
endif ()
