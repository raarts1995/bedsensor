#include "gpio.h"
#define TAG "GPIO"

TimerHandle_t gpio_timer;

gpioState_t gpio_apLedState;
gpioState_t gpio_staLedState;

uint8_t gpio_slowCnt;

/*
	Initialize the required GPIO pins and start the timer for blinking
*/
void gpio_init() {
	gpio_install_isr_service(0);

	gpio_config_t btn = {0};
	btn.mode = GPIO_MODE_INPUT;
	btn.pin_bit_mask = (1ULL<<GPIO_BUTTON);
	btn.pull_up_en = 1;
	gpio_config(&btn);

	gpio_config_t leds = {0};
	leds.mode = GPIO_MODE_INPUT_OUTPUT;
	leds.pin_bit_mask = ((1ULL<<GPIO_AP_LED) | (1ULL<<GPIO_STA_LED));
	gpio_config(&leds);

	gpio_setState(GPIO_AP_LED, GPIO_OFF);
	gpio_setState(GPIO_STA_LED, GPIO_OFF);

	gpio_startTimer();
}

/*
	Get the current state of the button
	Returns 1 if the button is pressed
			0 if not
*/
bool gpio_getButtonState() {
	return !gpio_get_level(GPIO_BUTTON); //btn uses pullup, so inverted logic
}

/*
	Set the state of the leds
*/
void gpio_setState(gpioDevice_t dev, gpioState_t state) {
	switch (dev) {
		case GPIO_AP_LED:
			gpio_apLedState = state;
			if (state < GPIO_BLINK)
				gpio_set_level(GPIO_AP_LED, state);
			break;
		case GPIO_STA_LED:
			gpio_staLedState = state;
			if (state < GPIO_BLINK)
				gpio_set_level(GPIO_STA_LED, state);
			break;
		default:
			break;
	}
}

/*
	GPIO timer callback function
	Sets the state of the LEDs opposite of the current state
*/
void gpio_timerTask(TimerHandle_t tmr) {
	if ((gpio_apLedState == GPIO_BLINK) || 
		((gpio_apLedState == GPIO_BLINK_SLOW) && (gpio_slowCnt % GPIO_SLOW_CNT == 0)))
			gpio_set_level(GPIO_AP_LED, !gpio_get_level(GPIO_AP_LED));

	if ((gpio_staLedState == GPIO_BLINK) || 
		((gpio_staLedState == GPIO_BLINK_SLOW) && (gpio_slowCnt % GPIO_SLOW_CNT == 0)))
		gpio_set_level(GPIO_STA_LED, !gpio_get_level(GPIO_STA_LED));

	gpio_slowCnt = (gpio_slowCnt + 1) % GPIO_SLOW_CNT;
}

/*
	Starts the gpio timer
*/
void gpio_startTimer() {
	gpio_timer = xTimerCreate(
		"gpio timer", //timer name
		GPIO_TIMEOUT/portTICK_PERIOD_MS, //timer period
		pdTRUE, //autoreload
		NULL, //timer ID
		gpio_timerTask //callback function
		);
	if (xTimerStart(gpio_timer, 0) != pdPASS)
		ESP_LOGE(TAG, "Failed to start gpio timer");
}

/*
	Stops the GPIO timer
*/
void gpio_stopTimer() {
	if (xTimerDelete(gpio_timer, 0) != pdPASS)
		ESP_LOGE(TAG, "Failed to delete gpio timer");
}

/*
	Configure SD Card detect pin
*/
void gpio_configureSDCardDetect() {
	gpio_configurePin(GPIO_SD_CD, GPIO_MODE_INPUT);
}

/*
	Detect if an SD card is inserted
*/
bool gpio_SDCardDetected() {
	return gpio_getPin(GPIO_SD_CD);
}

/*
	Configure a GPIO pin
*/
void gpio_configurePin(gpioDevice_t dev, gpio_mode_t mode) {
	gpio_config_t cd = {0};
	cd.mode = mode;
	cd.pin_bit_mask = (1ULL<<dev);
	gpio_config(&cd);
}

/*
	Set the state of a GPIO pin
*/
void gpio_setPin(gpioDevice_t dev, bool state) {
	gpio_set_level(dev, state);
}

/*
	Get the state of a GPIO pin
*/
bool gpio_getPin(gpioDevice_t dev) {
	return gpio_get_level(dev);
}

/*
	Attach an interrupt to a GPIO pin
*/
void gpio_attachInterrupt(gpioDevice_t dev, gpio_int_type_t intType, void (*fn) (void*)) {
	gpio_set_intr_type(dev, intType);
	gpio_isr_handler_add(dev, fn, (void*)dev);
}

/*
	Detach a previously attached interrupt from a GPIO pin
*/
void gpio_detachInterrupt(gpioDevice_t dev) {
	gpio_isr_handler_remove(dev);
}
