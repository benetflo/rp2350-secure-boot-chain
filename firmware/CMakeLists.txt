cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(firmware)

pico_sdk_init()

add_executable(firmware main.c)

target_link_libraries(firmware pico_stdlib hardware_gpio)

pico_add_extra_outputs(firmware)

target_link_options(firmware PRIVATE -Wl,--section-start=.text=0x10040000)
