#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"

#include "esp_err.h"
#include "esp_log.h"

#include "config.h"
#include "spiffs.h"
#include "gpio.h"
#include "wifi.h"
#include "nvsStorage.h"

#define TAG "Main"

void app_main()
{
	nvsStorage_init();
	spiffs_init();
	gpio_init();
	wifi_init();
}
