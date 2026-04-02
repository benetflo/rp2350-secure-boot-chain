#include "hardware/gpio.h"
#include "pico/time.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include <stdint.h>
#include <stdio.h>

#include "modules.h"
#include "config.h"

#define FIRMWARE_B          0x101C0000
#define LED_PIN 15

void core_1_callback(void);

int main () {

    stdio_init_all();
    //sleep_ms(5000);

    multicore_lockout_victim_init();
    multicore_launch_core1(core_1_callback);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (1) 
    {
        gpio_put(LED_PIN, 1); sleep_ms(500);
        gpio_put(LED_PIN, 0); sleep_ms(500);
    }
}

void core_1_callback (void)
{
    uint32_t my_addr = (uint32_t)&main;
    uint32_t running_partition = (my_addr >= FIRMWARE_B) ? 1 : 0;
    
    printf("Core 1: Running partition %u\n", running_partition);
    
    // create url for request
    char url[64];
    snprintf(url, sizeof(url), "/firmware?partition=%u&version=%d", running_partition, FIRMWARE_VERSION);

    while (1)
    {
        if (wifi_connect(WIFI_SSID, WIFI_PASSWORD) == 0)
        {
            printf("Core 1: Connected to WiFi\n");
            int result = http_connect(HTTP_SERVER_HOST, url);
            if (result != 0)
            {
                printf("Core 1: OTA failed or server unreachable, result=%d\n", result);
            }
        }
        sleep_ms(10000); // 10 seconds
    }
}