cmake_minimum_required(VERSION 3.15)

include(config.cmake)

project(${PROJECT_NAME} C)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(GitAddSubmodule)

add_executable(${EXECUTABLE_NAME})

add_git_submodule(DIRECTORY thirdparty/ascon-c
                PROPAGATE ON
                PATCHES "patches/ascon-fix.patch")
add_subdirectory(src)

# Needed for correct Ascon linking
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13.0")
    target_link_options(${EXECUTABLE_NAME} PRIVATE $<$<CONFIG:DEBUG>:-fsanitize=address>)
endif ()
target_link_libraries(${EXECUTABLE_NAME} PUBLIC ${ASCON_LIB_NAME})
