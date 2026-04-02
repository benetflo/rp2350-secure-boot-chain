# Pico2 Secure Bootchain

*Before building...*
## Requirements
- arm-none-eabi-gcc
- picotool
- openssl
- python3
- Pico SDK at `/home/benji/pico-sdk` (or set `PICO_SDK_PATH`) *FIX THIS PATH*

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

=====================================================


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



# Setup and run HTTP server
```
cd http_server
sudo apt install ruby ruby-bundler
bundle install
ruby main.rb
```
