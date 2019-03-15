#ifndef WIFISCAN_H
#define WIFISCAN_H

#include "wifi.h"

#define WIFI_SCAN_RUNNING -1
#define WIFI_SCAN_FAILED -2

int16_t wifi_scanNetworks(bool async, bool showHidden, bool passive, uint32_t msPerChannel);
void wifi_scanDone();
uint16_t wifi_scanComplete();
void wifi_clearScanResults();
wifi_ap_record_t *wifi_getScanInfoByIndex(uint16_t i);
bool wifi_getNetworkInfo(uint16_t i, char **ssid, uint8_t *encType, int32_t *rssi, uint8_t **bssid, int32_t *channel);
char *wifi_SSID(uint16_t i);
wifi_auth_mode_t wifi_encryptionType(uint16_t i);
int32_t wifi_RSSI(uint16_t i);
uint8_t *wifi_BSSID(uint16_t i);
int32_t wifi_channel(uint16_t i);

#endif