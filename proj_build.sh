#!/bin/bash
set -e
rm -rf build
mkdir -p uf2

echo "===================================================" 
echo "===================================================" 
echo "============= Building Bootloader... =============="
echo "===================================================" 
echo "===================================================" 
cd bootloader
make clean
make
cp bootloader.uf2 ../uf2
cd ..

echo "===================================================" 
echo "===================================================" 
echo "============== Building Firmware... ==============="
echo "===================================================" 
echo "===================================================" 

PICO_SDK_PATH=/home/benji/pico-sdk cmake -B build
cmake --build build

cp build/firmware/firmware.uf2 uf2/
