#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include "esp_err.h"
#include "esp_log.h"

#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "esp_http_server.h"

#include "config.h"
#include "wifi.h"

#define SERVER_BASEPATH "/spiffs"

#define SERVER_TIMEOUT 10 * 60 * 1000 //ms = 10 minutes

//Max length a file path can have on storage
#define SERVER_MAX_FILENAME (ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN)

#define CHECK_FILE_EXT(filename, ext) (strcasecmp(&filename[strlen(filename) - sizeof(ext) + 1], ext) == 0)

//timeout timer functions
void server_timeoutFunction(TimerHandle_t tmr);
void server_startTimer();
void server_resetTimer();
void server_deleteTimer();

esp_err_t initSpiffs();
esp_err_t server_start();
void server_stop();
char* server_setContentType(char* filename);
esp_err_t server_getSSIDList(httpd_req_t* req);
esp_err_t server_getCurrentSSID(httpd_req_t* req);
esp_err_t server_connect(httpd_req_t* req);
esp_err_t server_getWifiState(httpd_req_t* req);
esp_err_t server_sendFile(httpd_req_t* req);
char* server_getUrlQuery(httpd_req_t* req);
esp_err_t server_getArg(char* buf, char* key, char* value, size_t len);
void server_decodeURL(char* data, uint16_t strLen);
#endif