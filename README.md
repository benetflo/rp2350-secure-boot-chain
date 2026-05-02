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

```
┌──────────────┐
│     ROM      │   Immutable root of trust
└───────┬──────┘
        │ verifies
        ▼
┌──────────────┐
│  Bootloader  │   Verifies signatures, metadata, header
└───────┬──────┘
        │ verifies
        ▼
┌──────────────┐
│   Firmware   │   Runs only if authenticated
└──────────────┘
```

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

# Setup

## 1. Clone this repository
```
git clone --recurse-submodules https://github.com/benetflo/pico2-secure-bootchain.git
```

## 2. Install Pico SDK and add to PATH
```
cd ~
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init
echo 'export PICO_SDK_PATH=~/pico-sdk' >> ~/.bashrc
source ~/.bashrc
```

## 3. Install Picotool
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

## 4. Create a config.h file in the root directory of this repo
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

## 5. Setup and run HTTP server
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

## 1. Create bootloader signing keys
*(Signing of the bootloader is done during the build stage)*
```
openssl ecparam -name secp256k1 -genkey -noout -out bootloader_private_key.pem
openssl ec -in bootloader_private_key.pem -pubout -out bootloader_public_key.pem
```

## 2. Create firmware signing keys
```
openssl genpkey -algorithm ed25519 -out firmware_private_key.pem
openssl pkey -in firmware_private_key.pem -pubout -out firmware_public_key.pem
```

## 3. Build the project
```
make
```

## 4. Verify everything before programming OTP  
Flash the bootloader and firmware first to ensure everything works correctly.

### Verify that the bootloader key hash matches
```
openssl ec -in bootloader_private_key.pem -pubout -outform DER | tail -c 65 | tail -c 64 | sha256sum
python3 -c "import json; otp=json.load(open('otp.json')); print(''.join(f'{b:02x}' for b in otp['bootkey0']))"
```

## 5. Program OTP and burn the bootloader key permanently
```
sudo picotool otp load otp.json
```

# Programming OTP for rollback protection

## Programming minimum firmware version for rollback protection in OTP. 
The first available memory address recommended for user content by the RP2350 datasheet was used (row 0x0c0). Setting bit 0 to 1 encodes a minimum firmware version of 1 using a thermometer code, the bootloader counts the number of set bits (popcount) to determine the minimum allowed version.
```
sudo picotool otp set 0x0c0 0x0001
```

## Updating to a new firmware version  
Burn the next bit if you want to permanently block older versions. For version two this would look like this:
```
sudo picotool otp set 0x0c0 0x0003
```

## Optional production‑hardening (not used in this project)

The RP2350 includes several additional security features intended for production‑grade secure‑boot deployments.  
These are **not enabled in this project**, since this is an educational implementation and development access is required.  
However, they are recommended when locking down a device for real‑world use.

- **KEY_INVALID** —> revoke unused boot keys and prevent new keys from being added later  
- **Disable USB/UART boot modes** —> reduces attack surface by removing alternative boot paths  
- **Disable debug access** (`DEBUG_DISABLE` / `SECURE_DEBUG_DISABLE`) —> prevents attaching a debugger after deployment  
- **Enable glitch detector** —> provides hardware‑level protection against fault‑injection attacks  

These features are intentionally left disabled here to keep the device debuggable and to allow firmware development and testing.
