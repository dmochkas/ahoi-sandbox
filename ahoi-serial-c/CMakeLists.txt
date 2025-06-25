cmake_minimum_required(VERSION 3.10)
project(ahoi_serial C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(GitAddSubmodule)
include(sender-config.cmake)
include(receiver-config.cmake)

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
