# Raspberry Pi Pico 2 Secure Bootchain

This project was my Examensarbete/YH thesis for the IoT- & Embedded developer program at JENSEN Yrkeshögskola in Stockholm. This is my implementation of a secure boot chain on the Raspberry Pi Pico 2 (RP2350). 

# About the project
The system enforces a chain of trust from boot ROM to application firmware, ensuring that only authenticated and integrity-verified code is executed.

This project contains:

* A hardware root of trust based on the RP2350 boot ROM.
* A custom secure bootloader that is verified by the ROM bootloader via a public key stored in OTP. The key is signed using the secp256k1 algorithm. 
* Signed firmware images that are verified by the custom bootloader. The firmware is signed using the Ed25519 algorithm.
* A/B partitions (1.5 MB each) for fail-safe updates.
* Rollback protection using OTP, to prevent downgrading to vulnerable firmware versions.
* A simple HTTP server for OTA updates.
* Scripts for building, signing and flashing firmware.

*This project defends against:*
* Boot process tampering by ensuring that only trusted, signed bootloaders are executed.
* Firmware integrity attacks by ensuring that only signed firmware images are executed.
* Rollback attacks by enforcing firmware version control using OTP memory.
* Safe fallback to functional firmware by using A/B partitions, if errors were to occur during OTA updates or due to corrupted firmware images.

*This project does NOT defend against:*
* Physical attacks, such as hardware probing, memory extraction, or fault injection.
* Side-channel attacks, including timing or power analysis attacks.
* Attacks targeting the underlying hardware, such as exploitation of vulnerabilities in the boot ROM.
* Network-level attacks beyond the implemented security mechanisms of the update server.

This project does not make use of ARM TrustZone. However, it is recommended as an additional layer of security, as it enables the isolation of critical parts of the system, such as the bootloader, in a secure memory and execution environment.

The reason TrustZone is not used in this project is that the implementation was started without considering it. Therefore, integrating TrustZone at this stage would require a complete refactoring of the existing codebase. When comparing the effort required for such a refactor to the benefits it would provide, it was determined that it was not worth implementing within the scope and time constraints of this project. However, future work could consider integrating ARM TrustZone to further enhance system isolation and security.

# Guide for using this project

## Requirements
- arm-none-eabi-gcc
- picotool
- openssl
- python3
- Pico SDK
- ruby & ruby bundler

# SETUP

- Clone this repository
```
git clone --recurse-submodules https://github.com/benetflo/pico2-secure-bootchain.git
```

- Install Pico SDK and add to PATH
```
cd ~
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init
echo 'export PICO_SDK_PATH=~/pico-sdk' >> ~/.bashrc
source ~/.bashrc
```

- Install Picotool
```
cd ~
git clone --recurse-submodules https://github.com/raspberrypi/picotool.git
cd picotool
sudo apt update
sudo apt install cmake libusb-1.0-0-dev pkg-config git libmbedtls-dev
mkdir build && cd build
cmake .. -DPICO_SDK_PATH=~/pico-sdk
make -j$(nproc)
sudo make install

```
- Create a config.h file in the root directory of this repo
```
#ifndef CONFIG_H
#define CONFIG_H

#define FIRMWARE_VERSION 1

#define WIFI_SSID "your_ssid"
#define WIFI_PASSWORD "your_password"
#define PICO_SDK_PATH "path/to/pico-sdk"
#define HTTP_SERVER_HOST "ip_address"
#define HTTP_SERVER_PORT 4567

#endif
```

# Setup and run HTTP server
```
cd http_server
sudo apt install ruby ruby-bundler
bundle install
ruby main.rb
```


========================================================================
# CRITICAL SECTION, ERRORS HERE COULD BRICK SOME FEATURES OF SECURE BOOT
========================================================================

> **DISCLAIMER:** This guide is provided as-is for educational purposes. I take no responsibility 
> for bricked devices, lost keys, or any other damages resulting from following these instructions. 
> Do your own research before burning OTP — it is a permanent and irreversible operation.


## Activating secure boot on RP2350.

- Create keys (signing of bootloader is done during build stage)
```
openssl ecparam -name secp256k1 -genkey -noout -out bootloader_private_key.pem
openssl ec -in bootloader_private_key.pem -pubout -out bootloader_public_key.pem
```

- Create and generate firmware keys while we are at it :)
```
openssl genpkey -algorithm ed25519 -out firmware_private_key.pem
openssl pkey -in firmware_private_key.pem -pubout -out firmware_public_key.pem
```

- Build project
```
bash proj_build.sh
```

### NOTE: FLASH BOOTLOADER AND FIRMWARE TO CHECK THAT EVERYTHING IS WORKING BEFORE PROGRAMMING OTP.

- Verify that key-hash is correct (should be the same)
```
openssl ec -in bootloader_private_key.pem -pubout -outform DER | tail -c 65 | tail -c 64 | sha256sum
python3 -c "import json; otp=json.load(open('otp.json')); print(''.join(f'{b:02x}' for b in otp['bootkey0']))"
```


- Program OTP and burn bootloader key PERMANENTLY
```
sudo picotool otp load otp.json
```

- If using WSL on Windows like me, expose RP2350 via usbipd before programming OTP
```
winget install usbipd
usbipd list
usbipd bind --busid <busid>
usbipd attach --wsl --busid <busid>
```

## Programming OTP for rollback protection

- Programming minimum firmware version for rollback protection in OTP. The first available memory address recommended for user content by the RP2350 datasheet was used (row 0x0c0). Setting bit 0 to 1 encodes a minimum firmware version of 1 using a thermometer code, the bootloader counts the number of set bits (popcount) to determine the minimum allowed version.
```
sudo picotool otp set 0x0c0 0x0001
```

NOTE: Each time you want to flash a new firmware image with a new version:
- Update firmware versions in "config.h" and "http_server/versions.json".
- Burn the next bit in OTP register IF you want to permanently block earlier versions from running.
For version two this would look like this
```
sudo picotool otp set 0x0c0 0x0003
```
