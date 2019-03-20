#include "gpio.h"

void gpio_init() {
	
}

bool gpio_getButton() {

}

void gpio_setState(gpioDevice_t dev, gpioState_t state) {

}

void gpio_timerTask(TimerHandle_t tmr) {

}

/*
	Starts the server timeout timer
	Should only be called when the server starts
*/
void gpio_startTimer() {
	TimerHandle_t gpioTimer = xTimerCreate(
		"gpio timer", //timer name
		GPIO_TIMEOUT/portTICK_PERIOD_MS, //timer period
		pdTRUE, //autoreload
		NULL, //timer ID
		gpio_timerTask //callback function
		);
	if (xTimerStart(gpioTimer, 0) != pdPASS)
		ESP_LOGE(TAG, "Failed to start gpio timer");
}