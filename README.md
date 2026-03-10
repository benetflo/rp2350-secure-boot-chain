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

- Install openssl and generate keys
```
sudo apt install openssl
openssl genpkey -algorithm ed25519 -out private_key.pem
openssl pkey -in private_key.pem -pubout -out public_key.pem
```
