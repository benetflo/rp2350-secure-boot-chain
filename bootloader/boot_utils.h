#ifndef BOOT_UTILS_H
#define BOOT_UTILS_H

#include <stdint.h>

#define GREEN_LED 16
#define RED_LED 17
#define YELLOW_LED 18

void gpio_init_output(uint32_t pin);

void gpio_high(uint32_t pin);
void gpio_low(uint32_t pin);

void delay(uint32_t count);

#endif