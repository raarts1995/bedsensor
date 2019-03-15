#include "wifi.h"

/* FreeRTOS event group to signal when we are connected*/
EventGroupHandle_t wifiEventGroup;

uint16_t wifi_connectRetries = 0;

esp_err_t wifi_eventHandler(void *ctx, system_event_t *event) {
	switch(event->event_id) {
		//STA events
		case SYSTEM_EVENT_STA_START:
			ESP_LOGI(TAG, "event sta start");
			if (!(wifi_getStatusBit(WIFI_SCANNING_BIT)))
				wifi_connect();
			break;
		case SYSTEM_EVENT_STA_GOT_IP:
			ESP_LOGI(TAG, "event got ip: %s",
					 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
			
			wifi_setStatusBit(WIFI_CONNECTED_BIT);
			wifi_clearStatusBit(WIFI_CONNECTING_BIT);
			break;

		//triggered when:
		//  - esp_wifi_disconnect() is called
		//  - connecting to a network failed
		case SYSTEM_EVENT_STA_DISCONNECTED:
			ESP_LOGI(TAG, "event sta disconnected");
			wifi_clearStatusBit(WIFI_CONNECTED_BIT);

			//do not retry if wifi_disconnect was called
			if (wifi_getStatusBit(WIFI_DISCONNECT_CALLED_BIT)) {
				ESP_LOGI(TAG, "Disconnect called. Don't reconnect");
				wifi_clearStatusBit(WIFI_DISCONNECT_CALLED_BIT);
				wifi_clearStatusBit(WIFI_CONNECTING_BIT);
			}
			else {
				wifi_connect();
			}
			break;

		//AP events
		case SYSTEM_EVENT_AP_STACONNECTED:
			ESP_LOGI(TAG, "station: "MACSTR" join, AID = %d",
					 MAC2STR(event->event_info.sta_connected.mac),
					 event->event_info.sta_connected.aid);
			break;
		case SYSTEM_EVENT_AP_STADISCONNECTED:
			ESP_LOGI(TAG, "station: "MACSTR" leave, AID = %d",
					 MAC2STR(event->event_info.sta_disconnected.mac),
					 event->event_info.sta_disconnected.aid);

			if (wifi_getStatusBit(WIFI_CONNECTED_BIT)) {
				ESP_LOGI(TAG, "User disconnected from AP: stopping server");
				wifi_stopAP();
			}
			break;

		//scan events
		case SYSTEM_EVENT_SCAN_DONE:
			ESP_LOGI(TAG, "Scanning done");
			wifi_scanDone();
			break;
		default:
			break;
	}
	return ESP_OK;
}

void wifi_setStatusBit(EventBits_t bit) {
	ESP_LOGI(TAG, "Set bit: %s", wifi_statusBitToString(bit));
	xEventGroupSetBits(wifiEventGroup, bit);
}

void wifi_clearStatusBit(EventBits_t bit) {
	ESP_LOGI(TAG, "Clr bit: %s", wifi_statusBitToString(bit));
	xEventGroupClearBits(wifiEventGroup, bit);
}

bool wifi_getStatusBit(EventBits_t bit) {
	if (xEventGroupGetBits(wifiEventGroup) & bit)
		return true;
	return false;
}

bool wifi_waitForStatusBit(EventBits_t bit, uint32_t timeout_ms) {
	if (xEventGroupWaitBits(wifiEventGroup, bit, pdFALSE, pdTRUE, timeout_ms/portTICK_PERIOD_MS) & bit)
		return true;
	return false;
}

char *wifi_statusBitToString(EventBits_t bit) {
	switch (bit) {
		case WIFI_CONNECTED_BIT:
			return "WIFI_CONNECTED_BIT";
		case WIFI_CONNECTING_BIT:
			return "WIFI_CONNECTING_BIT";
		case WIFI_CONNECTING_FAILED_BIT:
			return "WIFI_CONNECTING_FAILED_BIT";
		case WIFI_DISCONNECT_CALLED_BIT:
			return "WIFI_DISCONNECT_CALLED_BIT";
		case WIFI_AP_ACTIVE:
			return "WIFI_AP_ACTIVE";
		case WIFI_SCAN_DONE_BIT:
			return "WIFI_SCAN_DONE_BIT";
		case WIFI_SCANNING_BIT:
			return "WIFI_SCANNING_BIT";
		default:
			return "UNDEFINED";
	}
}

void wifi_init() {
	wifiEventGroup = xEventGroupCreate();

	tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_init(wifi_eventHandler, NULL));

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
}

/*
	Monitors wifi connection attempts
	Only for internal use
	To connect to a wifi network use wifi_connectSTA
	
	The function attempts to connect to a network for a maximum of MAX_CONNECT_RETRIES
	If this fails, wifi_disconnect is called and the CONNECTING_FAILED_BIT is raised
*/
void wifi_connect() {
	//not the first attempt: increase attempt counter
	if (wifi_getStatusBit(WIFI_CONNECTING_BIT)) {
		wifi_connectRetries++;
	}

	//first attempt: set clear CONNECTING_FAILED, CONNECTING en DISCONNECT_CALLED bit
	else {
		wifi_setStatusBit(WIFI_CONNECTING_BIT);
		wifi_clearStatusBit(WIFI_CONNECTING_FAILED_BIT);
		wifi_connectRetries = 0;
	}
	
	//max retries: clear CONNECTING bit and set CONNECTING_FAILED bit
	//             calls wifi_disconnect() to clear wifi connection settings
	//             starts AP
	if (wifi_connectRetries >= WIFI_MAX_CONNECT_RETRIES) {
		ESP_LOGE(TAG, "WiFi connection attempts exceeded");
		wifi_clearStatusBit(WIFI_CONNECTING_BIT);
		wifi_disconnect();
		wifi_setStatusBit(WIFI_CONNECTING_FAILED_BIT);
		wifi_startAP();
	}

	//retry connecting
	else {
		ESP_LOGI(TAG, "WiFi connection attempt nr %d", wifi_connectRetries+1);
		ESP_ERROR_CHECK(esp_wifi_connect());	
	}
}

/*
	Disconnect from the wifi network

	Sets the DISCONNECT_CALLED bit only if connected to an AP so the event handler doesn't attempt to reconnect
	Clears the CONNECTING_FAILED bit so the correct state is returned to the user when wifi_connectState is called
*/
void wifi_disconnect() {
	ESP_LOGI(TAG, "WiFi disconnect");

	if (wifi_getStatusBit(WIFI_CONNECTED_BIT))
		wifi_setStatusBit(WIFI_DISCONNECT_CALLED_BIT);
	wifi_clearStatusBit(WIFI_CONNECTING_FAILED_BIT);

	ESP_ERROR_CHECK(esp_wifi_disconnect());
}

wifiState_t wifi_connectState() {
	if (wifi_getStatusBit(WIFI_CONNECTED_BIT))
		return WIFI_STATE_CONNECTED;
	if (wifi_getStatusBit(WIFI_CONNECTING_BIT))
		return WIFI_STATE_CONNECTING;
	if (wifi_getStatusBit(WIFI_CONNECTING_FAILED_BIT))
		return WIFI_STATE_CONNECTING_FAILED;
	return WIFI_STATE_DISCONNECTED;
}

esp_err_t wifi_connectSTA(char *ssid, char *pass) {
	ESP_LOGI(TAG, "Starting STA");
	if (wifi_getStatusBit(WIFI_CONNECTED_BIT))
		wifi_disconnect();

	wifi_config_t wifiConfig = {};
	strcpy((char *)wifiConfig.sta.ssid, ssid);
	strcpy((char *)wifiConfig.sta.password, pass);
	wifi_enableMode(WIFI_MODE_STA);
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifiConfig));
	esp_err_t err = esp_wifi_start();
	if (err == ESP_OK) {
		ESP_LOGI(TAG, "WiFi started in STA mode. SSID: '%s', pass: '%s'", ssid, pass);
		wifi_connect();
	}
	else
		ESP_LOGE(TAG, "WiFi failed to start in STA mode");
	return err;
}

/*
	start the AP and the webserver afterwards
*/
esp_err_t wifi_startAP() {
	if (wifi_getStatusBit(WIFI_AP_ACTIVE)) {
		ESP_LOGI(TAG, "AP already active");
		return ESP_OK;
	}
	ESP_LOGI(TAG, "Starting AP");
	wifi_config_t wifiConfig = {
		.ap = {
			.ssid = BS_WIFI_SSID, 
			.password = BS_WIFI_PASS,
			.max_connection = BS_MAX_STA_CONN,
			.authmode = WIFI_AUTH_WPA_WPA2_PSK
		},
	};
	if (strlen(BS_WIFI_PASS) == 0) 
		wifiConfig.ap.authmode = WIFI_AUTH_OPEN;

	wifi_enableMode(WIFI_MODE_AP);
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifiConfig));
	esp_err_t err = esp_wifi_start();
	if (err == ESP_OK) {
		ESP_LOGI(TAG, "WiFi started in AP mode. SSID: '%s', pass: '%s'", BS_WIFI_SSID, BS_WIFI_PASS);
		server_start(); //start webserver
		wifi_setStatusBit(WIFI_AP_ACTIVE);
	}
	else
		ESP_LOGE(TAG, "WiFi failed to start in AP mode");
	return err;
}

void wifi_stopAP() {
	server_stop();
	wifi_disableMode(WIFI_MODE_AP);
	wifi_clearStatusBit(WIFI_AP_ACTIVE);
}

void wifi_enableMode(wifi_mode_t newMode) {
	wifi_mode_t mode;
	ESP_ERROR_CHECK(esp_wifi_get_mode(&mode));
	ESP_ERROR_CHECK(esp_wifi_set_mode(mode | newMode));
}

void wifi_disableMode(wifi_mode_t newMode) {
	wifi_mode_t mode;
	ESP_ERROR_CHECK(esp_wifi_get_mode(&mode));
	ESP_ERROR_CHECK(esp_wifi_set_mode(mode & ~newMode));
}
