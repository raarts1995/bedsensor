#include "espSystem.h"

#define TAG "ESP System"

char espSystem_macAddr[]= "xx:xx:xx:xx:xx:xx";

void espSystem_init() {
	ESP_LOGI(TAG, "Initializing esp system");
	memset(espSystem_macAddr, '\0', sizeof(espSystem_macAddr));
	uint8_t mac[6] = {0};
	esp_efuse_mac_get_default(mac);
	sprintf(espSystem_macAddr, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	ESP_LOGI(TAG, "MAC address: %s", espSystem_macAddr);
}

char* espSystem_getMacAddr() {
	return espSystem_macAddr;
}
