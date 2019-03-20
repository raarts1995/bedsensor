#ifndef GPIO_H
#define GPIO_H

#define GPIO_TIMEOUT 500

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include "config.h"

typedef enum {
	AP_LED,
	STA_LED
} gpioDevice_t;

typedef enum {
	OFF,
	ON,
	BLINK
} gpioState_t;

void gpio_init();
bool gpio_getButton();
void gpio_setState(gpioDevice_t dev, gpioState_t state);

#endif