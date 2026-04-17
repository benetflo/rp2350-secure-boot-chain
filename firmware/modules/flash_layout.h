#ifndef FLASH_LAYOUT_H
#define FLASH_LAYOUT_H

#include <stdint.h>

/*************************************************************************************
 * NOTE:
 * This flash layout is not secret in a cryptographic sense, but it is considered
 * sensitive information. Knowledge of the exact memory layout can significantly
 * simplify reverse engineering and analysis of the system.
 *
 * The file is included in the source tree because it is required to build and
 * link the firmware. In this repository it is provided solely for the purpose of
 * demonstrating the system architecture as part of the exam project. It is not
 * intended for use in a production environment or for distribution outside this
 * example project.
 *************************************************************************************/

#define FIRMWARE_A          0x10040000
#define FIRMWARE_B          0x101C0000

#define FW_MAGIC 0xB00710AD

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
    uint32_t magic;
    uint32_t size;
    uint16_t version;
} fw_header_t; //max 256 bytes

#endif