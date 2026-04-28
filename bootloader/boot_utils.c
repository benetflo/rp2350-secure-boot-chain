#include <stdint.h>
#include "boot_utils.h"

#define SIO_BASE         0xD0000000
#define GPIO_OE_SET      (*(volatile uint32_t *)(SIO_BASE + 0x038))
#define GPIO_OE_CLR      (*(volatile uint32_t *)(SIO_BASE + 0x040))
#define GPIO_OUT_SET     (*(volatile uint32_t *)(SIO_BASE + 0x018))
#define GPIO_OUT_CLR     (*(volatile uint32_t *)(SIO_BASE + 0x020))

#define IO_BANK0_BASE    0x40028000
#define PADS_BANK0_BASE  0x40038000
#define OTP_DATA_RAW_BASE 0x40134000
#define ROLLBACK_OTP_ROW 0x0c0 // first row in OTP adress map that documentation recommends for user content

void gpio_init_output(uint32_t pin)
{
    GPIO_OE_CLR  = (1u << pin);
    GPIO_OUT_CLR = (1u << pin);
    // IOMUX - FUNCSEL = 5 (SIO)
    *(volatile uint32_t *)(IO_BANK0_BASE + pin * 8 + 4) = 5;
    // Pad isolation = 0
    *(volatile uint32_t *)(PADS_BANK0_BASE + 0x04 + pin * 0x04) &= ~(1u << 8);
    GPIO_OE_SET  = (1u << pin);
}

void gpio_high(uint32_t pin) { GPIO_OUT_SET = (1u << pin); }
void gpio_low(uint32_t pin)  { GPIO_OUT_CLR = (1u << pin); }

void delay(uint32_t count)
{
    while(count--) __asm__ volatile("nop");
}

static uint32_t popcount16(uint16_t val)
{
    uint32_t count = 0;
    while (val)
    {
        count += val & 1;
        val >>= 1;
    }
    return count;
}

static uint16_t otp_read_min_version()
{
    volatile uint16_t *row = (volatile uint16_t *)(OTP_DATA_RAW_BASE + (ROLLBACK_OTP_ROW * 4)); // first row in recommended user content. 
    
    if (popcount16(*row & 0xFFFF) == 0)
    {
        gpio_init_output(16);
        gpio_high(16);
    }
    
    return popcount16(*row & 0xFFFF); // returns the amount of 1's in the 16-bit register
}

int check_firmware_version(uint8_t partition_flag)
{
    fw_header_t * temp_hdr;
    
    if (partition_flag == 0)
    {
        temp_hdr = (fw_header_t*)FIRMWARE_A_HEADER;
    }
    else
    {
        temp_hdr = (fw_header_t*)FIRMWARE_B_HEADER;
    }

    uint16_t fw_version = temp_hdr->version;
    uint16_t min_version = otp_read_min_version();

    if (fw_version < min_version) // ROLLBACK PROTECTION
    {
        return -1;
    }

    return 0; // OK
}

int firmware_validate_size(uint32_t fw_size, fw_header_t * fw_hdr)
{
    // Validate header
    if (fw_hdr->magic != FW_MAGIC)
    {
        gpio_high(RED_LED);
        return -1;            
    }

    // Size must be > 0
    if (fw_size == 0)
    {
        gpio_high(RED_LED);
        return -1;
    }

    // Size must be 4-byte aligned
    if (fw_size & 0x3)
    {
        gpio_high(RED_LED);
        return -1;            
    }

    // Firmware + signature must fit in slot
    if (fw_size + 64 > SLOT_SIZE)
    {
        gpio_high(RED_LED);
        return -1;
    }
    return 0;
}