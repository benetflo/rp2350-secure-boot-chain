#!/bin/bash

set -e

cmake -B build
cmake --build build

mkdir -p uf2
cp build/bootloader/bootloader.uf2 uf2/
cp build/firmware/firmware.uf2 uf2/
echo "UF2 files copied to uf2/"