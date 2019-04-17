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

#define TAG "Main"

//init application
void app_main() {
	espSystem_init();
	nvsStorage_init();
	spiffs_init();
	gpio_init();
	wifi_init();
	rtcTime_init();
	sd_init();

	xTaskCreatePinnedToCore(&aws_testTask, "aws test task", 8192, NULL, 5, NULL, 1);
}
