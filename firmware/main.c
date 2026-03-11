#include <stdint.h>
#include "hardware/gpio.h"
#include "pico/time.h"
#include "modules.h"

#define LED_PIN 15

int main() {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (1) {
        gpio_put(LED_PIN, 1); sleep_ms(500);
        gpio_put(LED_PIN, 0); sleep_ms(500);
    }
}
