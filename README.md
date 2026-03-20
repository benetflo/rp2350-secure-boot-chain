# Pico2 Secure Bootchain

## *Embedded System Security Architecture*

### *Threat Model*

#### *Assets*
- Firmware Integrity
- Confidential cryptographic keys
- Device functionality and reliability
- The OTA update mechanism

The primary assets in this system are the integrity of the firmware, the cryptographic keys used for signing firmware updates, and the reliability of the device. If these assets are compromised, an attacker could execute arbitrary code or disrupt device functionality.

#### *Attackers*
#### *Attacker Capabilities*
#### *Assumptions*

### *Attack Scenarios*

#### *Malicious Firmware Installation*
#### *OTA Firmware Tampering*
#### *Physical Firmware Replacement*
#### *Supply Chain Attack*

### *Security Design*

#### *Root of Trust*
#### *Secure Boot*
#### *Firmware Signing*
#### *Secure OTA updates*
#### *Securing the Build and Update Chain*


*Before building...*
## Requirements
- arm-none-eabi-gcc
- picotool
- openssl
- python3
- Pico SDK at `/home/benji/pico-sdk` (or set `PICO_SDK_PATH`) *FIX THIS PATH*

- Clone this repository
```
git clone --recurse-submodules https://github.com/benetflo/pico2-secure-bootchain.git
```

- Install Pico SDK



- Download Picotool
```
sudo apt install cmake libusb-1.0-0-dev pkg-config git
git clone https://github.com/raspberrypi/picotool.git
cd picotool
mkdir build && cd build
cmake .. -DPICO_SDK_PATH=/home/benji/pico-sdk
make
sudo make install
picotool version
```

- Install uf2conv (NOTE: included in the repo so not really needed)
```
wget https://raw.githubusercontent.com/microsoft/uf2/master/utils/uf2conv.py
wget https://raw.githubusercontent.com/microsoft/uf2/master/utils/uf2families.json
```

- Install openssl and generate firmware keys
```
sudo apt install openssl
openssl genpkey -algorithm ed25519 -out private_key.pem
openssl pkey -in private_key.pem -pubout -out public_key.pem
```

# Activating secure boot on RP2350 (CRITICAL SECTION, ERRORS HERE COULD BRICK SECURE BOOT)

> **DISCLAIMER:** This guide is provided as-is for educational purposes. I take no responsibility 
> for bricked devices, lost keys, or any other damages resulting from following these instructions. 
> Do your own research before burning OTP — it is a permanent and irreversible operation.

- Create keys (signing of bootloader is done during build stage)
```
openssl ecparam -name secp256k1 -genkey -noout -out bootloader_private_key.pem
openssl ec -in bootloader_private_key.pem -pubout -out bootloader_public_key.pem
```

- Build project
```
bash proj_build.sh
```

- Verify that key-hash is correct (should be the same)
```
openssl ec -in bootloader_private_key.pem -pubout -outform DER | tail -c 65 | tail -c 64 | sha256sum
python3 -c "import json; otp=json.load(open('otp.json')); print(''.join(f'{b:02x}' for b in otp['bootkey0']))"
```

- If using WSL on Windows, expose RP2350 via usbipd before programming OTP
```
winget install usbipd
usbipd list
usbipd bind --busid <busid>
usbipd attach --wsl --busid <busid>
```

- Program OTP and burn bootloader key PERMANENTLY
```
sudo picotool otp load otp.json
```


# Create a config.h file in the root directory of this repo
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