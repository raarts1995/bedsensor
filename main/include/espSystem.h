#ifndef ESPSYSTEM_H
#define ESPSYSTEM_H

#include <stdio.h>
#include <string.h>

#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"

// waardes ingesteld via make menuconfig

#define BS_WIFI_SSID		CONFIG_ESP_WIFI_SSID
#define BS_WIFI_PASS		CONFIG_ESP_WIFI_PASSWORD
#define BS_MAX_STA_CONN		CONFIG_MAX_STA_CONN

void espSystem_init();
char* espSystem_getMacAddr();

#endif