#include "sd.h"

#define TAG "SD"

esp_err_t sd_init() {
	ESP_LOGI(TAG, "Initializing SD card using SPI");

	if (!gpio_getState(GPIO_SD_CD)) {
		ESP_LOGE(TAG, "No SD card inserted");
		return ESP_FAIL;
	}

	sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	sdspi_slot_config_t slotConfig = SDSPI_SLOT_CONFIG_DEFAULT();

	slotConfig.gpio_miso = GPIO_SD_MISO;
	slotConfig.gpio_mosi = GPIO_SD_MOSI;
	slotConfig.gpio_sck = GPIO_SD_SCK;
	slotConfig.gpio_cs = GPIO_SD_CS;

	esp_vfs_fat_sdmmc_mount_config_t mountConfig = {
		.format_if_mount_failed = false,
		.max_files = 8,
		.allocation_unit_size = 16 * 1024
	};

	sdmmc_card_t* card;
	esp_err_t err = esp_vfs_fat_sdmmc_mount(SD_BASEPATH, &host, &slotConfig, &mountConfig, &card);
	if (err != ESP_OK) {
		if (err == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount filesystem");
		}
		else {
			ESP_LOGE(TAG, "Failed to initialize SD card (%s)", esp_err_to_name(err));
		}
		return ESP_FAIL;
	}

	sdmmc_card_print_info(stdout, card);
	return ESP_OK;
}