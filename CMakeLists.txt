cmake_minimum_required(VERSION 3.16)

set(PICO_PLATFORM rp2350)
set(PICO_BOARD pico2)

include(cmake/pico_sdk_import.cmake)

project(ThesisProject C CXX ASM)

pico_sdk_init()

add_subdirectory(bootloader)
