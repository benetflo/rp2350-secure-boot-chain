#include <stdint.h>

#define FIRMWARE_BASE 0x10040000

int main(void) {
    uint32_t sp  = *(volatile uint32_t *)(FIRMWARE_BASE + 0);
    uint32_t rst = *(volatile uint32_t *)(FIRMWARE_BASE + 4);

    __asm__ volatile (
        "msr msp, %0\n"
        "bx  %1\n"
        : : "r"(sp), "r"(rst)
    );

    while(1);
}