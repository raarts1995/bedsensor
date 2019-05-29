#ifndef GPIO_H
#define GPIO_H

#define GPIO_TIMEOUT 500
#define GPIO_SLOW_CNT 4

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include "esp_log.h"
#include "esp_err.h"

#include "driver/gpio.h"

#include "espSystem.h"

typedef enum {
	GPIO_BUTTON = 4,
	GPIO_AP_LED = 16,
	GPIO_STA_LED = 17,


	//SD card pins (SD card driver does not support bus sharing (yet?))
	GPIO_SD_MISO = 19,
	GPIO_SD_MOSI = 23,
	GPIO_SD_SCK = 18,
	GPIO_SD_CS = 5,
	GPIO_SD_CD = 21, //36,

	//common SPI bus pins
	GPIO_MISO = 27,
	GPIO_MOSI = 13,
	GPIO_SCK = 14,
	
	//ADC pins
	GPIO_ADC_CS = 26
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

void gpio_configurePin(gpioDevice_t dev, gpio_mode_t mode);
void gpio_setPin(gpioDevice_t dev, bool state);
bool gpio_getPin(gpioDevice_t dev);
void gpio_attachInterrupt(gpioDevice_t dev, gpio_int_type_t intType, void (*fn) (void*));
void gpio_detachInterrupt(gpioDevice_t dev);

#endif