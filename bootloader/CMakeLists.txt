cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(bootloader)

pico_sdk_init()

add_executable(bootloader main.c)

target_link_libraries(bootloader pico_stdlib hardware_gpio)

pico_add_extra_outputs(bootloader)


