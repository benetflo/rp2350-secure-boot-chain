#ifndef MODULES_H
#define MODULES_H

#include <stdint.h>

#define FIRMWARE_A          0x10040000
#define FIRMWARE_B          0x101C0000

#define FIRMWARE_A_FLASH_OFFSET      0x00040000
#define FIRMWARE_A_HEADER_OFFSET     0x001BFF00
#define FIRMWARE_B_FLASH_OFFSET      0x001C0000
#define FIRMWARE_B_HEADER_OFFSET     0x0033FF00   

#define METADATA_FLASH_OFFSET    0x00350000  // 0x10350000 - 0x10000000
#define METADATA_ADDR            (XIP_BASE + METADATA_FLASH_OFFSET)

#define SLOT_SIZE 0x180000  // 1.5MB

typedef struct {
    uint32_t active_partition;
    uint32_t magic;
} OTA_METADATA_T;

typedef struct {
    uint32_t size;
    uint32_t version;
    uint8_t  signature[64];
} fw_header_t; //max 256 bytes

int wifi_connect(char * ssid, char * password);
int http_connect(char * host, char * url_request);


#endif