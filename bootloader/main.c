#include <stdint.h>
#include <stddef.h>
#include "mem.h"
#include "boot_utils.h"
#include "Hacl_Ed25519.h"
#include "Hacl_Hash_SHA2.h"

static uint8_t public_key[32] = 
{
    0xa5, 0x7a, 0xa9, 0xd3, 0x9c, 0xb1, 0x2a, 0xd8,
    0x91, 0xeb, 0xb3, 0x1d, 0x9b, 0xb5, 0x7a, 0x98,
    0xc4, 0xe3, 0xad, 0x5c, 0x0e, 0xda, 0x31, 0xb5,
    0x99, 0xb0, 0xf4, 0xad, 0x6a, 0x66, 0x94, 0x3e
};

static uint8_t partition_flag = 0; // default == A

int main (void) {
      
    gpio_init_output(YELLOW_LED);
    gpio_init_output(RED_LED);

    gpio_high(YELLOW_LED);

    OTA_METADATA_T * meta = (OTA_METADATA_T *)METADATA_ADDR;

    uint32_t firmware_base;
    fw_header_t * fw_hdr;

    if (meta->magic != 0xDEADBEEF) // first boot
    {
        partition_flag = 0;
        firmware_base = FIRMWARE_A;
        fw_hdr = (fw_header_t *)FIRMWARE_A_HEADER;
    }
    else
    {
        if (meta->active_partition == 1) 
        {
            partition_flag = 1;
            firmware_base   = FIRMWARE_B;
            fw_hdr = (fw_header_t *)FIRMWARE_B_HEADER;
        } 
        else 
        {
            partition_flag = 0;
            firmware_base   = FIRMWARE_A;
            fw_hdr = (fw_header_t *)FIRMWARE_A_HEADER;
        }
    }
    
    if (check_firmware_version(partition_flag) != 0) // ROLLBACK PROTECTION
    {
        gpio_high(RED_LED);
        while(1);
    }

    uint32_t firmware_size = fw_hdr->size;
    uint32_t firmware_version = fw_hdr->version;
    uint8_t * firmware = (uint8_t *)firmware_base;
    uint8_t * fw_sig = firmware + firmware_size;

    uint8_t fw_hash[32];
    Hacl_Hash_SHA2_hash_256(fw_hash, firmware, firmware_size); // hash firmware to optimize Ed25519 verification
    
    if (!Hacl_Ed25519_verify(public_key, 32, fw_hash, fw_sig))   // verifies signature from hashed firmware (faster)
    {
        gpio_high(RED_LED);

        // signature fail → try other slot
        if (firmware_base == FIRMWARE_A) 
        {
            partition_flag = 1;
            firmware_base   = FIRMWARE_B;
            fw_hdr = (fw_header_t *)FIRMWARE_B_HEADER;
        } 
        else 
        {
            partition_flag = 0;
            firmware_base   = FIRMWARE_A;
            fw_hdr = (fw_header_t *)FIRMWARE_A_HEADER;
        }

        if (check_firmware_version(partition_flag) != 0) // ROLLBACK PROTECTION
        {
            gpio_high(RED_LED);
            while(1);
        }

        firmware_size = fw_hdr->size;
        firmware_version = fw_hdr->version;
        firmware = (uint8_t *)firmware_base;
        fw_sig = firmware + firmware_size;

        Hacl_Hash_SHA2_hash_256(fw_hash, firmware, firmware_size);

        if (!Hacl_Ed25519_verify(public_key, 32, fw_hash, fw_sig)) {
            // båda slots ogiltiga → stanna i rött
            gpio_low(YELLOW_LED);
            gpio_high(RED_LED);
            while (1);
        }
        gpio_low(RED_LED);
    }

    // Light up GREEN LED to indicate that firmware was accepted
    gpio_init_output(GREEN_LED);
    gpio_low(YELLOW_LED);
    gpio_high(GREEN_LED);
    delay(5000000);

    // Firmware's vector table starts at FIRMWARE_BASE
    // First 4 bytes = stack pointer value (an address in RAM e.g. 0x20082000)
    // Next 4 bytes  = reset handler address (an address in flash e.g. 0x100400e5)
    uint32_t sp  = *(volatile uint32_t *)(firmware_base + 0);           
    uint32_t rst = *(volatile uint32_t *)(firmware_base + 4);           

    __asm__ volatile (
        "msr msp, %0\n"                 // Set the stack pointer to the value we read from firmware.This must be done before jumping so firmware has a valid stack
        "bx  %1\n"                      // Jump to firmware's reset handler - From here SDK takes over - initializes clocks, memory and calls main()
        : : "r"(sp), "r"(rst)           // Load sp and rst into CPU registers before executing assembly
    );

    while(1);       // this will never be reached, firmware has taken over
}