#include "nvsStorage.h"

nvs_handle bsNvs;
bool nvsStorage_opened = false;

esp_err_t nvsStorage_init() {
	//Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	return ret;
}

esp_err_t nvsStorage_open() {
	ESP_LOGI(TAG, "Opening nvs handle");
	//opening nvs using TAG name
	return nvs_open(TAG, NVS_READWRITE, &bsNvs);
}

void nvsStorage_close() {
	ESP_LOGI(TAG, "Closing nvs handle");
	nvs_close(bsNvs);
}

esp_err_t nvsStorage_getString(char* key, char* value, size_t len) {
	if (!nvsStorage_opened) {
		nvsStorage_opened = true;
		nvsStorage_open();
	}
	return nvs_get_str(bsNvs, key, value, &len);
	
}

esp_err_t nvsStorage_setString(char* key, char* value) {
	if (!nvsStorage_opened) {
		nvsStorage_opened = true;
		nvsStorage_open();
	}
	esp_err_t err = nvs_set_str(bsNvs, key, value);
	if (err == ESP_OK)
		return nvs_commit(bsNvs);
	return err;
}