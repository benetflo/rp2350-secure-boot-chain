cmake_minimum_required(VERSION 3.13)

# Project root directory
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(A_BASE_ADDR 0x10040000)
set(A_FIRMWARE_HEADER 0x101BFF00)
set(A_PARTITION_NUM 0)

set(B_BASE_ADDR 0x101C0000)
set(B_FIRMWARE_HEADER 0x1033FF00)
set(B_PARTITION_NUM 1)

# Define output dirs
set(OUT_DIR ${ROOT_DIR}/out/firmware)
set(SIGNED_DIR ${OUT_DIR}/signed)
set(UNSIGNED_DIR ${OUT_DIR}/unsigned)

set(PICO_BOARD pico2_w CACHE STRING "Board type")
include(../cmake/pico_sdk_import.cmake)

project(pico2_bootchain)
pico_sdk_init()

include_directories(${ROOT_DIR})

file(STRINGS "${ROOT_DIR}/config.h" FIRMWARE_VERSION REGEX "#define FIRMWARE_VERSION ([0-9]+)")
string(REGEX MATCH "[0-9]+" FIRMWARE_VERSION "${FIRMWARE_VERSION}")

# ============================================================
#  FUNCTION TO GENERATE FIRMWARE TARGETS
# ============================================================
function(add_firmware TARGET PARTITION_ID LINKER_SCRIPT BASE_ADDR HEADER_ADDR)
    add_executable(${TARGET}
        main.c
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/network.c
    )

    target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/modules)

    target_link_libraries(${TARGET}
        pico_stdlib
        hardware_gpio
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_http
        hardware_flash
        pico_multicore
        hardware_watchdog
    )

    target_compile_definitions(${TARGET} PRIVATE PARTITION_ID="${PARTITION_ID}")
    pico_set_linker_script(${TARGET} ${LINKER_SCRIPT})

    pico_enable_stdio_usb(${TARGET} 1) # DISABLE FOR PRODUCTION
    pico_enable_stdio_uart(${TARGET} 1) # DISABLE FOR PRODUCTION

    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND arm-none-eabi-objcopy -O binary ${TARGET}.elf ${TARGET}.bin

        COMMAND python3 ${ROOT_DIR}/tools/sign_firmware.py
            ${TARGET}.bin
            ${TARGET}.sig
            ${TARGET}_signed.bin
            ${ROOT_DIR}/firmware_private_key.pem

        COMMAND python3 ${ROOT_DIR}/tools/extract_firmware_size.py
            ${TARGET}.bin
            ${TARGET}_size.bin
            ${FIRMWARE_VERSION}

        COMMAND python3 ${ROOT_DIR}/tools/uf2conv.py
            ${TARGET}_signed.bin
            -o fw_${PARTITION_ID}.uf2
            --family RP2350_ARM_S
            --base ${BASE_ADDR}

        COMMAND python3 ${ROOT_DIR}/tools/uf2conv.py
            ${TARGET}_size.bin
            -b ${HEADER_ADDR}
            -o size_${PARTITION_ID}.uf2
            -f RP2350_ARM_S
            -c

        COMMAND python3 ${ROOT_DIR}/tools/merge_uf2.py
            fw_${PARTITION_ID}.uf2
            size_${PARTITION_ID}.uf2
            ${TARGET}.uf2

        COMMAND python3 ${ROOT_DIR}/tools/uf2conv.py
            ${TARGET}.bin
            -o unsigned_${TARGET}.uf2
            --family RP2350_ARM_S
            --base ${BASE_ADDR}

        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building and signing ${TARGET}"

        # Create output directories
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SIGNED_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${UNSIGNED_DIR}

        # Copy signed output
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.uf2
            ${SIGNED_DIR}/${TARGET}_v${FIRMWARE_VERSION}.uf2

        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_signed.bin
            ${SIGNED_DIR}/${TARGET}_v${FIRMWARE_VERSION}.bin

        # Copy unsigned output
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_BINARY_DIR}/unsigned_${TARGET}.uf2
            ${UNSIGNED_DIR}/${TARGET}_v${FIRMWARE_VERSION}.uf2

        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.bin
            ${UNSIGNED_DIR}/${TARGET}_v${FIRMWARE_VERSION}.bin


    )
endfunction()

# ============================================================
#  CREATE FIRMWARE A + B
# ============================================================

add_firmware(
    firmware_a
    ${A_PARTITION_NUM}
    ${CMAKE_CURRENT_SOURCE_DIR}/firmware_a.ld
    ${A_BASE_ADDR}
    ${A_FIRMWARE_HEADER}
)

add_firmware(
    firmware_b
    ${B_PARTITION_NUM}
    ${CMAKE_CURRENT_SOURCE_DIR}/firmware_b.ld
    ${B_BASE_ADDR}
    ${B_FIRMWARE_HEADER}
)