#ifndef GPIO_H
#define GPIO_H

#define GPIO_TIMEOUT 500
#define GPIO_SLOW_CNT 4

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include "esp_log.h"
#include "esp_err.h"

#include "driver/gpio.h"

#include "config.h"

typedef enum {
	GPIO_BUTTON = 4,
	GPIO_AP_LED = 16,
	GPIO_STA_LED = 17,

	GPIO_SD_MISO = 19,
	GPIO_SD_MOSI = 23,
	GPIO_SD_SCK = 18,
	GPIO_SD_CS = 5,
	GPIO_SD_CD = 36
} gpioDevice_t;

typedef enum {
	GPIO_OFF,
	GPIO_ON,
	GPIO_BLINK,
	GPIO_BLINK_SLOW
} gpioState_t;

void gpio_init();
bool gpio_getState(gpioDevice_t dev);
bool gpio_getButtonState();
void gpio_setState(gpioDevice_t dev, gpioState_t state);
void gpio_timer_task(TimerHandle_t tmr);
void gpio_startTimer();
void gpio_stopTimer();

void gpio_configureSDCardDetect();
bool gpio_SDCardDetected();

#endif