#include <stdint.h>
#include "hardware/gpio.h"
#include "pico/time.h"

#define LED_PIN         15
#define FIRMWARE_BASE   0x10040000

int main() {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    /* 3 blinkar = bootloader kör */
    for (int i = 0; i < 3; i++) {
        gpio_put(LED_PIN, 1); sleep_ms(200);
        gpio_put(LED_PIN, 0); sleep_ms(200);
    }
    sleep_ms(300);

    /* Läs SP och reset-handler från firmware vektortabell */
    uint32_t sp  = *(volatile uint32_t *)(FIRMWARE_BASE + 0);
    uint32_t rst = *(volatile uint32_t *)(FIRMWARE_BASE + 4);

    /* Hoppa till firmware */
    __asm__ volatile (
        "msr msp, %0\n"
        "bx  %1\n"
        : : "r"(sp), "r"(rst)
    );

    while(1);
}
