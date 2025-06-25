cmake_minimum_required(VERSION 3.10)
project(ahoi_serial C)

set(CMAKE_C_STANDARD 11)

# Configuration for Linux
add_executable(sender src/sender.c src/serial_utils.c)
add_executable(receiver src/receiver.c src/serial_utils.c)