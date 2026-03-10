#!/bin/bash

set -e
rm -rf build
PICO_SDK_PATH=/home/benji/pico-sdk cmake -B build
cmake --build build
mkdir -p uf2

# Sign bootloader
cd build/bootloader
picotool seal --sign bootloader.elf signed_bootloader.elf ../../bootloader_private_key.pem ../../otp.json
arm-none-eabi-objcopy -O binary signed_bootloader.elf signed_bootloader.bin
python3 ../../tools/uf2conv.py signed_bootloader.bin -o bootloader.uf2 --family RP2350_ARM_S --base 0x10000000
cd ../..

cp build/bootloader/bootloader.uf2 uf2/
cp build/firmware/firmware.uf2 uf2/
echo "UF2 files copied to uf2/"