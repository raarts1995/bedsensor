#include "sd.h"

#define TAG "SD"

sdmmc_card_t* card;

FILE* sd_filePtr;

bool sd_mounted = false;

void sd_init() {
	ESP_LOGI(TAG, "Initializing SD card");

	gpio_configureSDCardDetect();

	sd_mounted = false;
	sd_filePtr = NULL;

	if (gpio_SDCardDetected())
		sd_mount();
}

esp_err_t sd_mount() {
	if (sd_mounted) {
		ESP_LOGI(TAG, "Already mounted");
		return ESP_OK;
	}
	ESP_LOGI(TAG, "Mounting SD card");

	sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	sdspi_slot_config_t slotConfig = SDSPI_SLOT_CONFIG_DEFAULT();

	slotConfig.gpio_miso = GPIO_SD_MISO;
	slotConfig.gpio_mosi = GPIO_SD_MOSI;
	slotConfig.gpio_sck = GPIO_SD_SCK;
	slotConfig.gpio_cs = GPIO_SD_CS;

	esp_vfs_fat_sdmmc_mount_config_t mountConfig = {
		.format_if_mount_failed = false,
		.max_files = 4,
		.allocation_unit_size = 16 * 1024
	};

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
	sd_mounted = true;

	sd_printCardInfo();
	return ESP_OK;

}

void sd_unmount() {
	if (sd_mounted) {
		ESP_LOGI(TAG, "Unmounting SD card");
		ESP_ERROR_CHECK(esp_vfs_fat_sdmmc_unmount());
		card = NULL;
		sd_mounted = false;
	}
}

void sd_printCardInfo() {
	if (!gpio_SDCardDetected() || !sd_mounted)
		return;

	char* type = (card->ocr & SD_OCR_SDHC_CAP) ? "SDHC/SDXC" : "SDSC";
	ESP_LOGI(TAG, "Type: %s", type);
	if (card->max_freq_khz < 1000)
		ESP_LOGI(TAG, "Max speed: %d kHz", card->max_freq_khz);
	else
		ESP_LOGI(TAG, "Max speed: %d MHz", card->max_freq_khz/1000);
	ESP_LOGI(TAG, "Size: %lluMB", ((uint64_t)card->csd.capacity) * card->csd.sector_size / (1024 * 1024));
}

/*
	Checks if the given file exists
*/
bool sd_fileExists(char* path) {
	if (!gpio_SDCardDetected() || !sd_mounted)
		return false;

	return fileIO_fileExists(SD_BASEPATH, path);
}

/*
	Returns the size of a file
*/
size_t sd_getFileSize(char* path) {
	if (!gpio_SDCardDetected() || !sd_mounted)
		return 0;

	return fileIO_getFileSize(SD_BASEPATH, path);
}

/*
	Opens the requested file
	Returns true if the file is opened
			false if the file doesn't exist or opening failed
*/
bool sd_openFile(char* path) {
	if (!gpio_SDCardDetected() || !sd_mounted)
		return false;

	sd_closeFile();

	sd_filePtr = fileIO_openFile(SD_BASEPATH, path, "a");

	if (!sd_filePtr) {
		ESP_LOGE(TAG, "Failed to open file: %s", path);
		return false;
	}
	return true;
}

void sd_appendFile(char* data) {
	if (!gpio_SDCardDetected() || !sd_mounted)
		return;

	fileIO_writeFile(sd_filePtr, data);
}

/*
	Closes the opened file
*/
void sd_closeFile() {
	if (!gpio_SDCardDetected() || !sd_mounted)
		return;

	fileIO_closeFile(sd_filePtr);
	sd_filePtr = NULL;
}
