#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"

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
#include "algorithms.h"

#define TAG "Main"

/*
	Samples per Second
	     |-------------------------------|-------------------------------|
	     | Single cycle                  | Continuous cycle              |
	|----|---------------|---------------|---------------|---------------|
	|rate|LINEF=0 (60 Hz)|LINEF=1 (50 Hz)|LINEF=0 (60 Hz)|LINEF=1 (50 Hz)|
	|----|---------------|---------------|---------------|---------------|
	| 0  | 1             | 0,833         | -             | -             |
	| 1  | 2,5           | 2,08          | -             | -             |
	| 2  | 5             | 4,17          | -             | -             |
	| 3  | 10            | 8,33          | -             | -             |
	| 4  | 15            | 12,5          | 60            | 50            |
	| 5  | 30            | 25            | 120           | 100           |
	| 6  | 60            | 50            | 240           | 200           |
	| 7  | 120           | 100           | 480           | 400           |
	|----|---------------|---------------|---------------|---------------|
*/
#define ADC_RATE 5

#define avgMax 5
int32_t avgList[avgMax] = {0};
int avgCnt = 0;

//init application
void app_main() {
	espSystem_init();
	nvsStorage_init();
	spiffs_init();
	gpio_init();
	wifi_init();
	rtcTime_init();
	spi_init();
	sd_init();
	adc_init();
	alg_init();
	aws_init();
	
	adc_startConversion(ADC_RATE);
	adc_setInterrupt();

	/*TimerHandle_t tmr = xTimerCreate(
		"main tmr", //timer name
		(1000/SAMPLE_RATE)/portTICK_PERIOD_MS, //timer period
		pdTRUE, //autoreload
		NULL, //timer ID
		timerTick //callback function
		);
	if (xTimerStart(tmr, 0) != pdPASS)
		ESP_LOGE(TAG, "Failed to start timer");*/

	
}
