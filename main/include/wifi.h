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
#include "config.h"

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_CONNECTING_BIT BIT1
#define WIFI_CONNECTING_FAILED_BIT BIT2

#define WIFI_DISCONNECT_CALLED_BIT BIT3

#define WIFI_SCAN_DONE_BIT BIT4
#define WIFI_SCANNING_BIT BIT5

#define WIFI_MAX_CONNECT_RETRIES 3

typedef enum {
	WIFI_STATE_DISCONNECTED,
	WIFI_STATE_CONNECTING,
	WIFI_STATE_CONNECTING_FAILED,
	WIFI_STATE_CONNECTED
} wifiState_t;

esp_err_t wifi_eventHandler(void *ctx, system_event_t *event);
void wifi_setStatusBit(EventBits_t bit);
void wifi_clearStatusBit(EventBits_t bit);
bool wifi_getStatusBit(EventBits_t bit);
bool wifi_waitForStatusBit(EventBits_t bit, uint32_t timeout_ms);

void wifi_init();
void wifi_connect();
void wifi_disconnect();
wifiState_t wifi_connectState();
esp_err_t wifi_connectSTA(char *ssid, char *pass);
esp_err_t wifi_startAP(char *ssid, char *pass);
void wifi_stopAP();

void wifi_enableMode(wifi_mode_t newMode);
void wifi_disableMode(wifi_mode_t newMode);
#endif