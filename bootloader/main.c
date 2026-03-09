#include <stdint.h>

#define FIRMWARE_BASE 0x10040000                                        // Start address of firmware in flash

int main(void) {
    
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

    while(1);       // this will never be reached
}