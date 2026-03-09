cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(firmware)

pico_sdk_init()

add_executable(firmware main.c)

target_link_libraries(firmware pico_stdlib hardware_gpio)

add_custom_command(TARGET firmware POST_BUILD
    COMMAND arm-none-eabi-objcopy -O binary firmware.elf firmware.bin
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/uf2conv.py firmware.bin -o firmware.uf2 --family RP2350_ARM_S --base 0x10040000
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating firmware.uf2"
)

pico_set_linker_script(firmware ${CMAKE_CURRENT_SOURCE_DIR}/firmware.ld)
