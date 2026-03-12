#include <stdint.h>
#include <stddef.h>
#include "mem.h"
#include "Hacl_Ed25519.h"
#include "Hacl_Hash_SHA2.h"

#define FIRMWARE_BASE                       0x10040000                                        // Start address of firmware in flash.
#define FIRMWARE_HEADER                     0x1003FF00                                        // Where firmware size is stored. (Before firmware)

#define SIO_BASE         0xD0000000
#define GPIO_OE_SET      (*(volatile uint32_t *)(SIO_BASE + 0x038))
#define GPIO_OE_CLR      (*(volatile uint32_t *)(SIO_BASE + 0x03C))
#define GPIO_OUT_SET     (*(volatile uint32_t *)(SIO_BASE + 0x018))
#define GPIO_OUT_CLR     (*(volatile uint32_t *)(SIO_BASE + 0x01C))

#define IO_BANK0_BASE    0x40028000
#define PADS_BANK0_BASE  0x40038000

#define GREEN_LED 16
#define RED_LED 17

static void gpio_init_output(uint32_t pin)
{
    GPIO_OE_CLR  = (1u << pin);
    GPIO_OUT_CLR = (1u << pin);
    // IOMUX - FUNCSEL = 5 (SIO)
    *(volatile uint32_t *)(IO_BANK0_BASE + 0x04 + pin * 0x08) = 5;
    // Pad isolation = 0
    *(volatile uint32_t *)(PADS_BANK0_BASE + 0x04 + pin * 0x04) &= ~(1u << 8);
    GPIO_OE_SET  = (1u << pin);
}

static void gpio_high(uint32_t pin) { GPIO_OUT_SET = (1u << pin); }
static void gpio_low(uint32_t pin)  { GPIO_OUT_CLR = (1u << pin); }

static void delay(uint32_t count)
{
    while(count--) __asm__ volatile("nop");
}

int main (void) {
       
    static uint8_t public_key[32] = 
    {
        0xa5, 0x7a, 0xa9, 0xd3, 0x9c, 0xb1, 0x2a, 0xd8,
        0x91, 0xeb, 0xb3, 0x1d, 0x9b, 0xb5, 0x7a, 0x98,
        0xc4, 0xe3, 0xad, 0x5c, 0x0e, 0xda, 0x31, 0xb5,
        0x99, 0xb0, 0xf4, 0xad, 0x6a, 0x66, 0x94, 0x3e
    };
    
    uint32_t firmware_size = *(uint32_t *)FIRMWARE_HEADER;
    uint8_t * firmware = (uint8_t *)FIRMWARE_BASE;
    uint8_t * signature = firmware + firmware_size;

    uint8_t fw_hash[32];
    Hacl_Hash_SHA2_hash_256(fw_hash, firmware, firmware_size); // hash firmware to optimize Ed25519 verification
    
    if (!Hacl_Ed25519_verify(public_key, 32, fw_hash, signature))   // verifies signature from hashed firmware (faster)
    {
        // Light up RED LED to indicate that firmware was NOT accepted
        gpio_init_output(RED_LED);
        gpio_high(RED_LED);
        while(1);
    }

    // Light up GREEN LED to indicate that firmware was accepted
    gpio_init_output(GREEN_LED);
    gpio_high(GREEN_LED);
    delay(5000000);

    // Firmware's vector table starts at FIRMWARE_BASE
    // First 4 bytes = stack pointer value (an address in RAM e.g. 0x20082000)
    // Next 4 bytes  = reset handler address (an address in flash e.g. 0x100400e5)
    uint32_t sp  = *(volatile uint32_t *)(FIRMWARE_BASE + 0);           
    uint32_t rst = *(volatile uint32_t *)(FIRMWARE_BASE + 4);           

    __asm__ volatile (
        "msr msp, %0\n"                 // Set the stack pointer to the value we read from firmware.This must be done before jumping so firmware has a valid stack
        "bx  %1\n"                      // Jump to firmware's reset handler - From here SDK takes over - initializes clocks, memory and calls main()
        : : "r"(sp), "r"(rst)           // Load sp and rst into CPU registers before executing assembly
    );




    while(1);       // this will never be reached, firmware has taken over
}