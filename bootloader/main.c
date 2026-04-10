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

static uint8_t partition_flag = 0; // 0 = A, 1 = B


/*
* NOTE: the "goto" keyword is often consider a big "no no" in embedded programming, however the reason for using it here is because the bootloader is small and deterministic which improves the readability of the state-machine structure.
*/

int main(void)
{
    gpio_init_output(YELLOW_LED);
    gpio_init_output(RED_LED);

    gpio_high(YELLOW_LED);

    OTA_METADATA_T *meta = (OTA_METADATA_T *)METADATA_ADDR;

    uint32_t firmware_base;
    fw_header_t *fw_hdr;
    uint8_t tried_other = 0;

    // Pick initial partition
    if (meta->magic == 0xDEADBEEF)
    {
        if (meta->active_partition == 1)
        {
            partition_flag = 1;
            firmware_base  = FIRMWARE_B;
            fw_hdr         = (fw_header_t *)FIRMWARE_B_HEADER;
        }
        else
        {
            partition_flag = 0;
            firmware_base  = FIRMWARE_A;
            fw_hdr         = (fw_header_t *)FIRMWARE_A_HEADER;
        }
    }
    else
    {
        // first boot, partition A defualt
        partition_flag = 0;
        firmware_base  = FIRMWARE_A;
        fw_hdr         = (fw_header_t *)FIRMWARE_A_HEADER;

        // If invalid firmware header, stay in yellow (first boot)
        if (fw_hdr->magic != FW_MAGIC)
        {
            while (1)
            {
                gpio_high(YELLOW_LED);
            }
        }
    }

validate_slot:
    {
        // ROLLBACK PROTECTION
        if (check_firmware_version(partition_flag) != 0)
        {
            gpio_high(RED_LED);
            goto try_other_slot;
        }

        uint32_t firmware_size = fw_hdr->size;

        // validate firmware size
        if (firmware_validate_size(firmware_size, fw_hdr) != 0)
        {
            gpio_high(RED_LED);
            goto try_other_slot;
        }

        uint32_t firmware_version = fw_hdr->version;
        (void)firmware_version; // om du inte använder den vidare

        uint8_t *firmware = (uint8_t *)firmware_base;
        uint8_t *fw_sig   = firmware + firmware_size;

        uint8_t fw_hash[32];
        
        // hash firmware to optimize Ed25519 verification
        Hacl_Hash_SHA2_hash_256(fw_hash, firmware, firmware_size);

        // Verify firmware
        if (!Hacl_Ed25519_verify(public_key, 32, fw_hash, fw_sig))
        {
            gpio_high(RED_LED);
            goto try_other_slot;
        }

        // Boot firmware if everything is OK
        goto boot_firmware;
    }

try_other_slot:
    {
        if (tried_other)
        {
            // Both slots tested, stay in RED
            gpio_low(YELLOW_LED);
            gpio_high(RED_LED);
            while (1);
        }

        tried_other = 1;

        // Switch slot
        if (partition_flag == 0)
        {
            partition_flag = 1;
            firmware_base  = FIRMWARE_B;
            fw_hdr         = (fw_header_t *)FIRMWARE_B_HEADER;
        }
        else
        {
            partition_flag = 0;
            firmware_base  = FIRMWARE_A;
            fw_hdr         = (fw_header_t *)FIRMWARE_A_HEADER;
        }

        goto validate_slot;
    }

boot_firmware:
    {
        // Green LED == firmware accepted
        gpio_init_output(GREEN_LED);
        gpio_low(YELLOW_LED);
        gpio_high(GREEN_LED);
        delay(5000000);

        // Vector table in firmware
        uint32_t sp  = *(volatile uint32_t *)(firmware_base + 0);
        uint32_t rst = *(volatile uint32_t *)(firmware_base + 4);

        __asm__ volatile (
            "msr msp, %0\n"
            "bx  %1\n"
            :
            : "r"(sp), "r"(rst)
        );

        while (1);
    }
}
