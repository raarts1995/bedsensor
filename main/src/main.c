/* Simple WiFi Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"

#include "esp_err.h"
#include "esp_log.h"

#include "config.h"
#include "gpio.h"
#include "wifi.h"
#include "nvsStorage.h"


void rtosTimerTest(TimerHandle_t xTimer) {
	char *stateStr[] = {"Disconnected", "Connecting", "Connecting failed", "Connected"};
	ESP_LOGI(TAG, "WiFi state: %s", stateStr[wifi_connectState()]);
}

void app_main()
{
	nvsStorage_init();

	/*TimerHandle_t tmr = xTimerCreate(
		"test rtos timer", //timer name (not used by rtos)
		1000/portTICK_PERIOD_MS, //tick period
		pdTRUE, //autoreload
		NULL, //timer ID
		rtosTimerTest //callback function
		);
	if (xTimerStart(tmr, 0) != pdPASS)
		ESP_LOGE(TAG, "Timer start error");*/
	gpio_init();
	wifi_init();

}
