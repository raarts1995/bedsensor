#ifndef WIFI_H
#define WIFI_H

#include <string.h>

#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"

#include "wifiScan.h"
#include "server.h"
#include "nvsStorage.h"
#include "config.h"

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP?*/
#define WIFI_CONNECTED_BIT BIT0

//raised when wifi_connect() is called
//cleared event WIFI_STA_GOT_IP or WIFI_STA_DISCONNECTED
#define WIFI_CONNECTING_BIT BIT1

//raised when reconnect attempts > WIFI_MAX_CONNECT_RETRIES
//cleared when wifi_connect() or wifi_disconnect() is called
#define WIFI_CONNECTING_FAILED_BIT BIT2

//raised when wifi_disconnect() is called and WIFI_CONNECTED_BIT was set
//cleared on event WIFI_STA_DISCONNECTED
#define WIFI_DISCONNECT_CALLED_BIT BIT3

//raised when wifi_startAP() is called
//cleared when wifi_stopAP() is called
#define WIFI_AP_ACTIVE BIT4

#define WIFI_SCAN_DONE_BIT BIT5
#define WIFI_SCANNING_BIT BIT6

//maximum length of wifi ssid and pass
#define WIFI_MAX_SSID_LEN 32
#define WIFI_MAX_PASS_LEN 32

//maximum reconnect attempts
#define WIFI_MAX_CONNECT_RETRIES 3

typedef enum {
	WIFI_STATE_DISCONNECTED,
	WIFI_STATE_CONNECTING,
	WIFI_STATE_CONNECTING_FAILED,
	WIFI_STATE_CONNECTED
} wifiState_t;

esp_err_t wifi_eventHandler(void* ctx, system_event_t* event);
void wifi_setStatusBit(EventBits_t bit);
void wifi_clearStatusBit(EventBits_t bit);
bool wifi_getStatusBit(EventBits_t bit);
bool wifi_waitForStatusBit(EventBits_t bit, uint32_t timeout_ms);
char* wifi_statusBitToString(EventBits_t bit);

void wifi_init();
bool wifi_getWifiSettings();
void wifi_setWifiSettings();
char* wifi_getStoredSSID();
void wifi_connect();
void wifi_disconnect();
wifiState_t wifi_connectState();
esp_err_t wifi_connectSTA();
esp_err_t wifi_startAP();
void wifi_stopAP();

void wifi_enableMode(wifi_mode_t newMode);
void wifi_disableMode(wifi_mode_t newMode);
#endif