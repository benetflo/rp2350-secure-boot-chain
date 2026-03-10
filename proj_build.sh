#!/bin/bash
set -e
rm -rf build
rm -rf otp.json
mkdir -p uf2

echo "===================================================" 
echo "===================================================" 
echo "============= Building Bootloader... =============="
echo "===================================================" 
echo "===================================================" 
cd bootloader
make clean
make

# Sign bootloader
picotool seal --sign bootloader.elf signed_bootloader.elf ../bootloader_private_key.pem ../otp.json
arm-none-eabi-objcopy -O binary signed_bootloader.elf signed_bootloader.bin
python3 ../tools/uf2conv.py signed_bootloader.bin -o signed_bootloader.uf2 --family RP2350_ARM_S --base 0x10000000

cp signed_bootloader.uf2 ../uf2
cd ..

echo "===================================================" 
echo "===================================================" 
echo "============== Building Firmware... ==============="
echo "===================================================" 
echo "===================================================" 

PICO_SDK_PATH=/home/benji/pico-sdk cmake -B build
cmake --build build

cp build/firmware/firmware.uf2 uf2/

