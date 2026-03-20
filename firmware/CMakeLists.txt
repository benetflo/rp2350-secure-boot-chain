cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(firmware)

pico_sdk_init()


add_executable(firmware_a main.c ${CMAKE_CURRENT_SOURCE_DIR}/modules/network.c)

target_include_directories(firmware_a PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/modules)

target_link_libraries(firmware_a 
    pico_stdlib 
    hardware_gpio
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_http
    hardware_flash
    pico_multicore
    hardware_watchdog
)

target_compile_definitions(firmware_a PRIVATE PARTITION_ID="0")
pico_set_linker_script(firmware_a ${CMAKE_CURRENT_SOURCE_DIR}/firmware_a.ld)

pico_enable_stdio_usb(firmware_a 1)
pico_enable_stdio_uart(firmware_a 1)

add_custom_command(TARGET firmware_a POST_BUILD
    COMMAND arm-none-eabi-objcopy -O binary firmware_a.elf firmware_a.bin
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/sign_firmware.py firmware_a.bin firmware_a.sig firmware_a_signed.bin ${CMAKE_SOURCE_DIR}/firmware_private_key.pem
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/extract_firmware_size.py firmware_a.bin firmware_a_size.bin
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/uf2conv.py firmware_a_signed.bin -o fw_a.uf2 --family RP2350_ARM_S --base 0x10040000
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/uf2conv.py firmware_a_size.bin -b 0x1003FF00 -o size_a.uf2 -f RP2350_ARM_S -c
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/merge_uf2.py fw_a.uf2 size_a.uf2 firmware_a.uf2
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/uf2conv.py firmware_a.bin -o unsigned_firmware_a.uf2 --family RP2350_ARM_S --base 0x10040000
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating and signing firmware.uf2"
)

add_executable(firmware_b main.c ${CMAKE_CURRENT_SOURCE_DIR}/modules/network.c)

target_include_directories(firmware_b PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/modules)

target_link_libraries(firmware_b 
    pico_stdlib 
    hardware_gpio
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_http
    hardware_flash
    pico_multicore
    hardware_watchdog
)

pico_set_linker_script(firmware_b ${CMAKE_CURRENT_SOURCE_DIR}/firmware_b.ld)
target_compile_definitions(firmware_b PRIVATE PARTITION_ID="1")

pico_enable_stdio_usb(firmware_b 1)
pico_enable_stdio_uart(firmware_b 1)

add_custom_command(TARGET firmware_b POST_BUILD
    COMMAND arm-none-eabi-objcopy -O binary firmware_b.elf firmware_b.bin
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/sign_firmware.py firmware_b.bin firmware_b.sig firmware_b_signed.bin ${CMAKE_SOURCE_DIR}/firmware_private_key.pem
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/extract_firmware_size.py firmware_b.bin firmware_b_size.bin
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/uf2conv.py firmware_b_signed.bin -o fw_b.uf2 --family RP2350_ARM_S --base 0x101C0000
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/uf2conv.py firmware_b_size.bin -b 0x101BFF00 -o size_b.uf2 -f RP2350_ARM_S -c
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/merge_uf2.py fw_b.uf2 size_b.uf2 firmware_b.uf2
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/uf2conv.py firmware_b.bin -o unsigned_firmware_b.uf2 --family RP2350_ARM_S --base 0x101C0000
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating and signing firmware.uf2"
)

