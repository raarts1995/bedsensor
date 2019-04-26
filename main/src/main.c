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

#define SAMPLE_RATE 50 //hz

/*
	Samples per Second
	|----|---------------|---------------|
	|rate|LINEF=0 (60 Hz)|LINEF=1 (50 Hz)|
	|----|---------------|---------------|
	| 0  | 1             | 0,833         |
	| 1  | 2,5           | 2,08          |
	| 2  | 5             | 4,17          |
	| 3  | 10            | 8,33          |
	| 4  | 15            | 12,5          |
	| 5  | 30            | 25            |
	| 6  | 60            | 50            |
	| 7  | 120           | 100           |
	|----|---------------|---------------|
*/
#define ADC_RATE 7 

#define avgMax 5
int32_t avgList[avgMax] = {0};
int avgCnt = 0;

void timerTick(TimerHandle_t tmr) {
	if (!adc_running() || adc_ready()) {
		avgList[avgCnt] = adc_readData();
		int32_t avg = 0;
		for (int i = 0; i < avgMax; i++)
			avg += avgList[i];
		avg /= avgMax;
		
		printf("%d %d\n", avgList[avgCnt], avg);
		
		avgCnt = (avgCnt + 1) % avgMax;
		adc_startConversion(ADC_RATE);
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
	adc_startConversion(ADC_RATE);

	TimerHandle_t tmr = xTimerCreate(
		"main tmr", //timer name
		(1000/SAMPLE_RATE)/portTICK_PERIOD_MS, //timer period
		pdTRUE, //autoreload
		NULL, //timer ID
		timerTick //callback function
		);
	if (xTimerStart(tmr, 0) != pdPASS)
		ESP_LOGE(TAG, "Failed to start timer");

	xTaskCreatePinnedToCore(&aws_testTask, "aws test task", 8192, NULL, 5, NULL, 1);
}
