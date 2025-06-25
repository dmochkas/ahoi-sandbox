cmake_minimum_required(VERSION 3.10)
project(ahoi_serial C)

set(CMAKE_C_STANDARD 11)

# Configuration for Linux (termios)
add_executable(sender src/sender/sender.c src/serial_utils.c)
add_executable(receiver src/receiver/receiver.c src/serial_utils.c)
