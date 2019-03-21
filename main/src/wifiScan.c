#include "wifiScan.h"

uint16_t wifi_scanCount = 0;
wifi_ap_record_t* wifi_scanResult;

/*
	Scans for available wifi networks
	Returns the number of networks on success
			WIFI_SCAN_RUNNING when called in async mode
			WIFI_SCAN_FAILED on failure
*/
int16_t wifi_scanNetworks(bool async, bool showHidden, bool passive, uint32_t msPerChannel) {
	if (wifi_getStatusBit(WIFI_SCANNING_BIT)) {
		ESP_LOGI(TAG, "WiFi scan already running");
		return WIFI_SCAN_RUNNING;
	}
	wifi_setStatusBit(WIFI_SCANNING_BIT);
	wifi_enableMode(WIFI_MODE_STA);

	wifi_scan_config_t scanConfig;
	scanConfig.ssid = 0;
	scanConfig.bssid = 0;
	scanConfig.channel = 0;
	if (passive) {
		scanConfig.scan_type = WIFI_SCAN_TYPE_PASSIVE;
		scanConfig.scan_time.passive = msPerChannel;
	} 
	else {
		scanConfig.scan_type = WIFI_SCAN_TYPE_ACTIVE;
		scanConfig.scan_time.active.min = 100;
		scanConfig.scan_time.active.max = msPerChannel;
	}

	if (esp_wifi_scan_start(&scanConfig, false) == ESP_OK) {
		wifi_clearStatusBit(WIFI_SCAN_DONE_BIT);

		if (async)
			return WIFI_SCAN_RUNNING;

		if (wifi_waitForStatusBit(WIFI_SCAN_DONE_BIT, 10000))
			return wifi_scanCount;
	}

	wifi_clearStatusBit(WIFI_SCANNING_BIT);
	return WIFI_SCAN_FAILED;
}

/*
	Retrieves the scanned networks
	Is called when the SCAN_DONE bit raises
	Should not be called manually
*/
void wifi_scanDone() {
	esp_wifi_scan_get_ap_num(&wifi_scanCount);
	ESP_LOGI(TAG, "Scanning done: found %d networks", wifi_scanCount);
	if (wifi_scanCount) {
		if (wifi_scanResult)
			free(wifi_scanResult);
		wifi_scanResult = (wifi_ap_record_t*)calloc(wifi_scanCount, sizeof(wifi_ap_record_t));
		if (!wifi_scanResult || (esp_wifi_scan_get_ap_records(&wifi_scanCount, wifi_scanResult) != ESP_OK))
			wifi_scanCount = 0;
	}

	wifi_setStatusBit(WIFI_SCAN_DONE_BIT);
	wifi_clearStatusBit(WIFI_SCANNING_BIT);
}

/*
	Checks if the network scan has completed
	To be used when scanning asynchronally
	Returns the number of networks when the scan has finished
			WIFI_SCAN_RUNNING when the scan is still running
			WIFI_SCAN_FAILED on failure
*/
uint16_t wifi_scanComplete() {
	if (wifi_getStatusBit(WIFI_SCAN_DONE_BIT))
		return wifi_scanCount;
	if (wifi_getStatusBit(WIFI_SCANNING_BIT))
		return WIFI_SCAN_RUNNING;
	return WIFI_SCAN_FAILED;
}

/*
	Clears the scan results of the previous scan
*/
void wifi_clearScanResults() {
	if (wifi_scanResult)
		free(wifi_scanResult);
	wifi_scanResult = NULL;
	wifi_scanCount = 0;
	wifi_clearStatusBit(WIFI_SCAN_DONE_BIT);
}

/*
	Returns a scan record struct based on the given number
	Should only be called after scanNetworks() or scanComplete() is called without errors
*/
wifi_ap_record_t* wifi_getScanInfoByIndex(uint16_t i) {
	if ((wifi_scanResult == NULL) || (i >= wifi_scanCount))
		return NULL;
	return &wifi_scanResult[i];
}

/*
	Retrieves the network info of network i and stores it in the given variables
	Should only be called after scanNetworks() or scanComplete() is called without errors
*/
bool wifi_getNetworkInfo(uint16_t i, char** ssid, uint8_t* encType, int32_t* rssi, uint8_t** bssid, int32_t* channel) {
	wifi_ap_record_t* r = wifi_getScanInfoByIndex(i);
	if (r == NULL)
		return false;

	*ssid = (char*)r->ssid;
	*encType = r->authmode;
	*rssi = r->rssi;
	*bssid = r->bssid;
	*channel = r->primary;
	return true;
}

/*
	Returns the SSID of network i
	Should only be called after scanNetworks() or scanComplete() is called without errors
*/
char* wifi_SSID(uint16_t i) {
	wifi_ap_record_t* r = wifi_getScanInfoByIndex(i);
	if (r == NULL)
		return NULL;
	return (char*)r->ssid;
}

/*
	Returns the encryption type of network i
	Should only be called after scanNetworks() or scanComplete() is called without errors
*/
wifi_auth_mode_t wifi_encryptionType(uint16_t i) {
	wifi_ap_record_t* r = wifi_getScanInfoByIndex(i);
	if (r == NULL)
		return false;
	return r->authmode;
}

/*
	Returns the network strength (RSSI) of network i
	Should only be called after scanNetworks() or scanComplete() is called without errors
*/
int32_t wifi_RSSI(uint16_t i) {
	wifi_ap_record_t* r = wifi_getScanInfoByIndex(i);
	if (r == NULL)
		return false;
	return r->rssi;
}

/*
	Returns the MAC address (BSSID) of network i
	Should only be called after scanNetworks() or scanComplete() is called without errors
*/
uint8_t* wifi_BSSID(uint16_t i) {
	wifi_ap_record_t* r = wifi_getScanInfoByIndex(i);
	if (r == NULL)
		return false;
	return r->bssid;
}

/*
	Returns the channel used by network i
	Should only be called after scanNetworks() or scanComplete() is called without errors
*/
int32_t wifi_channel(uint16_t i) {
	wifi_ap_record_t* r = wifi_getScanInfoByIndex(i);
	if (r == NULL)
		return false;
	return r->primary;
}