cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME "ascon-demo")
set(EXECUTABLE_NAME ${PROJECT_NAME})

project(${PROJECT_NAME} C)

add_executable(${EXECUTABLE_NAME})

add_subdirectory(src)