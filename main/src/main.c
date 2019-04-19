#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include "esp_system.h"

#include "esp_err.h"
#include "esp_log.h"

#include "espSystem.h"
#include "nvsStorage.h"
#include "spiffs.h"
#include "gpio.h"
#include "wifi.h"
#include "rtcTime.h"
#include "aws.h"
#include "sd.h"
#include "spi.h"
#include "max11206.h"

#define TAG "Main"

void timerTick(TimerHandle_t tmr) {
	if (!adc_running() || adc_ready()) {
		printf("%u\n", adc_readData()); //50sps
		//ESP_LOGI(TAG, "%u", adc_readData());
		adc_startConversion(7);
	}
	else
		ESP_LOGI(TAG, "adc still running");
}

//init application
void app_main() {
	espSystem_init();
	nvsStorage_init();
	spiffs_init();
	gpio_init();
	wifi_init();
	rtcTime_init();
	//sd_init();
	spi_init();
	adc_init();
	adc_startConversion(6);

	TimerHandle_t tmr = xTimerCreate(
		"main tmr", //timer name
		20/portTICK_PERIOD_MS, //timer period
		pdTRUE, //autoreload
		NULL, //timer ID
		timerTick //callback function
		);
	if (xTimerStart(tmr, 0) != pdPASS)
		ESP_LOGE(TAG, "Failed to start timer");

	xTaskCreatePinnedToCore(&aws_testTask, "aws test task", 8192, NULL, 5, NULL, 1);
}
