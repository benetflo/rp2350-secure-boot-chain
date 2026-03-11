cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(firmware)

pico_sdk_init()

add_executable(firmware main.c)

target_include_directories(firmware PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/modules)
target_link_libraries(firmware pico_stdlib hardware_gpio)

add_custom_command(TARGET firmware POST_BUILD
    COMMAND arm-none-eabi-objcopy -O binary firmware.elf firmware.bin
    COMMAND openssl pkeyutl -sign -inkey ${CMAKE_SOURCE_DIR}/firmware_private_key.pem -rawin -in firmware.bin -out firmware.sig
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/sign_firmware.py firmware.bin firmware.sig firmware_signed.bin
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/uf2conv.py firmware_signed.bin -o fw.uf2 --family RP2350_ARM_S --base 0x10040000
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/uf2conv.py firmware_size.bin -b 0x1003FF00 -o size.uf2 -f RP2350_ARM_S -c
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/merge_uf2.py fw.uf2 size.uf2 firmware.uf2
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/check_sign_firmware.py firmware_signed.bin firmware_size.bin
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating and signing firmware.uf2"
)

pico_set_linker_script(firmware ${CMAKE_CURRENT_SOURCE_DIR}/firmware.ld)
