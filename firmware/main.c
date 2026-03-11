#include <stdint.h>
#include "hardware/gpio.h"
#include "pico/time.h"
#include "modules.h"
#include <stdio.h>
#include "pico/stdlib.h"

#define LED_PIN 15

int main() {

    stdio_init_all();
    sleep_ms(5000);

    if (wifi_connect("SSID", "PASSWORD") == 0)
    {
        printf("Connected to WiFi");
    }

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (1) {
        gpio_put(LED_PIN, 1); sleep_ms(500);
        gpio_put(LED_PIN, 0); sleep_ms(500);
    }
}
