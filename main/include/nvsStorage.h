#ifndef NVSSTORAGE_H
#define NVSSTORAGE_H
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "nvs.h"
#include "espSystem.h"

esp_err_t nvsStorage_init();
esp_err_t nvsStorage_open();
esp_err_t nvsStorage_getString(char* key, char* value, size_t len);
esp_err_t nvsStorage_setString(char* key, char* value);

#endif