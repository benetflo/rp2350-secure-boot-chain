#include <stdint.h>
#include "hardware/gpio.h"
#include "pico/time.h"
#include "modules.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "../config.h"

#define FIRMWARE_B          0x101C0000

#define LED_PIN 15

int main() {

    stdio_init_all();
    sleep_ms(5000);

    uint32_t my_addr = (uint32_t)&main;
    uint32_t running_partition = (my_addr >= FIRMWARE_B) ? 1 : 0;

    printf("Running partition %u\n", running_partition);

    // create url for request
    char url[64];
    snprintf(url, sizeof(url), "/firmware?partition=%u&version=%d", running_partition, FIRMWARE_VERSION);

    printf("Running partition %u\n", running_partition);

    if (wifi_connect(WIFI_SSID, WIFI_PASSWORD) == 0)
    {
        printf("Connected to WiFi");
        http_connect(HTTP_SERVER_HOST, url);
    }

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (1) 
    {
        gpio_put(LED_PIN, 1); sleep_ms(500);
        gpio_put(LED_PIN, 0); sleep_ms(500);
    }
}
