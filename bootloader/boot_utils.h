#ifndef BOOT_UTILS_H
#define BOOT_UTILS_H

#include <stdint.h>

#define FIRMWARE_A          0x10040000
#define FIRMWARE_A_HEADER   0x101BFF00
#define FIRMWARE_B          0x101C0000
#define FIRMWARE_B_HEADER   0x1033FF00
#define SLOT_SIZE           0x00180000
#define METADATA_ADDR       0x10350000

#define GREEN_LED 18
#define RED_LED 19
#define YELLOW_LED 21

typedef struct
{
    uint32_t active_partition; // 0 == A, 1 == B
    uint32_t magic;
} OTA_METADATA_T;

typedef struct {
    uint32_t size;
    uint32_t version;
    uint8_t  signature[64];
} fw_header_t; // max 256 bytes

void gpio_init_output(uint32_t pin);

void gpio_high(uint32_t pin);
void gpio_low(uint32_t pin);

void delay(uint32_t count);

int check_firmware_version(uint8_t partition_flag);

#endif