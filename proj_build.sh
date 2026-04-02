#!/bin/bash
set -e
rm -rf build
rm -rf otp.json
mkdir -p uf2
rm -f uf2/*

echo "===================================================" 
echo "===================================================" 
echo "============= Building Bootloader... =============="
echo "===================================================" 
echo "===================================================" 
cd bootloader
make clean
make

# Sign bootloader
picotool seal --sign build/bootloader.elf build/signed_bootloader.elf ../bootloader_private_key.pem ../otp.json
arm-none-eabi-objcopy -O binary build/signed_bootloader.elf build/signed_bootloader.bin
python3 ../tools/uf2conv.py build/signed_bootloader.bin -o build/signed_bootloader.uf2 --family RP2350_ARM_S --base 0x10000000

cp build/signed_bootloader.uf2 ../uf2
cp build/bootloader.uf2 ../uf2/unsigned_bootloader.uf2
cd ..

echo "===================================================" 
echo "===================================================" 
echo "============== Building Firmware... ==============="
echo "===================================================" 
echo "===================================================" 

PICO_SDK_PATH=$(grep '#define PICO_SDK_PATH' config.h | sed -E 's/#define PICO_SDK_PATH "(.*)"/\1/') cmake -B build
cmake --build build -- -j$(nproc)

cp build/firmware/firmware_a.uf2 uf2/signed_firmware_a.uf2
cp build/firmware/firmware_b.uf2 uf2/signed_firmware_b.uf2
cp build/firmware/unsigned_firmware_a.uf2 uf2/
cp build/firmware/unsigned_firmware_b.uf2 uf2/