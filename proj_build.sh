#!/bin/bash

set -e
rm -rf build
PICO_SDK_PATH=/home/benji/pico-sdk cmake -B build
cmake --build build
mkdir -p uf2
cp build/bootloader/bootloader.uf2 uf2/
cp build/firmware/firmware.uf2 uf2/
echo "UF2 files copied to uf2/"