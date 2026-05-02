# Secure Boot Chain for RP2350 with Signed Firmware, A/B Updates, and Rollback Protection

# About the project

This project was part of my Examensarbete/YH thesis for the IoT- & Embedded developer program at JENSEN Yrkeshögskola in Stockholm. This is my implementation of a secure boot chain on the Raspberry Pi Pico 2 (RP2350). The goal is to build a practical, working example of how a secure‑boot process can be designed and validated on a real device.

A custom bootloader verifies firmware signatures, checks metadata, enforces versioning rules, and manages an A/B update layout with rollback protection. The system ensures that only authenticated and integrity‑protected firmware is allowed to run.

The boot process is designed to handle realistic failure scenarios such as corrupted images, incomplete OTA updates, and invalid headers. All verification happens on‑device, and the OTA server is treated as an untrusted component. Firmware is only accepted if its signature, header, and version information are valid; otherwise, the bootloader falls back to the alternate slot or enters a safe error state.

This project is not meant to be a full production‑grade security solution, but rather a learning‑focused implementation that demonstrates the core concepts of secure boot, firmware authentication, safe OTA updates, and rollback protection in practice.

## What this project includes:

* ROM based Root of Trust.
* A custom bootloader, verified by the ROM bootloader using a public key stored in OTP.
* Signed firmware images, verified by custom bootloader before execution.
* A/B partitions (1,5 MB each), for firmware images.
* Rollback protection using OTP-stored minimum firmware version.
* A simple HTTP server for OTA updates.
* Scripts for building, signing and flashing firmware.

## Chain of Trust

The system enforces a continuous chain of trust from the immutable boot ROM to the application firmware. Each stage verifies the integrity and authenticity of the next stage, ensuring that only authorized and integrity‑checked code is executed on the device.

┌──────────────┐
│     ROM       │   Immutable root of trust
└───────┬──────┘
        │ verifies
        ▼
┌──────────────┐
│  Bootloader   │   Verifies signatures, metadata, header
└───────┬──────┘
        │ verifies
        ▼
┌──────────────┐
│   Firmware    │   Runs only if authenticated
└──────────────┘

# Security limitations

Since this is an educational first implementation of a secure boot chain, the system has been designed and tested within a defined threat model. Core security mechanisms such as secure boot, A/B partitions, rollback protection, and signature verification are implemented, but the system may still contain undiscovered implementation issues or vulnerabilities.

Testing has mainly focused on expected failure cases, such as invalid signatures and corrupted metadata. More advanced hardware attacks, like fault injection, side-channel analysis, and physical probing, are considered out of scope for this project and have not been evaluated.

The OTA server is intentionally kept minimal and not production-ready, and is only meant to demonstrate the secure boot chain, with all critical trust decisions enforced on the device itself.

### *Some of the security‑related tests in this project are demonstrated in this [video](https://youtu.be/aSPZq3tKgkI?si=sQaqq6llWd9MbziV):*

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

<br><br>

# CRITICAL SECTION, ERRORS HERE COULD BRICK SOME FEATURES OF SECURE BOOT

> **DISCLAIMER:** This guide is provided as-is for educational purposes. I take no responsibility 
> for bricked devices, lost keys, or any other damages resulting from following these instructions. 
> Do your own research before burning OTP — it is a permanent and irreversible operation.


# Activating secure boot on RP2350.

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
make
```

## NOTE: FLASH BOOTLOADER AND FIRMWARE TO CHECK THAT EVERYTHING IS WORKING BEFORE PROGRAMMING OTP.

- Verify that key-hash is correct (should be the same)
```
openssl ec -in bootloader_private_key.pem -pubout -outform DER | tail -c 65 | tail -c 64 | sha256sum
python3 -c "import json; otp=json.load(open('otp.json')); print(''.join(f'{b:02x}' for b in otp['bootkey0']))"
```


- Program OTP and burn bootloader key PERMANENTLY
```
sudo picotool otp load otp.json
```

# Programming OTP for rollback protection

- Programming minimum firmware version for rollback protection in OTP. The first available memory address recommended for user content by the RP2350 datasheet was used (row 0x0c0). Setting bit 0 to 1 encodes a minimum firmware version of 1 using a thermometer code, the bootloader counts the number of set bits (popcount) to determine the minimum allowed version.
```
sudo picotool otp set 0x0c0 0x0001
```

NOTE: Each time you want to flash a new firmware image with a new version:
- Burn the next bit in OTP register IF you want to permanently block earlier versions from running.
For version two this would look like this
```
sudo picotool otp set 0x0c0 0x0003
```

### Quick tip for WSL users
- If using WSL on Windows like me, expose RP2350 via usbipd before programming OTP. Make sure Pico is in BOOTSEL mode
```
winget install usbipd
usbipd list
usbipd bind --busid <busid>
usbipd attach --wsl --busid <busid>
```
