#include "sd.h"

#define TAG "SD"

sdmmc_card_t* card;

FILE* sd_csvFilePtr;
FILE* sd_logFilePtr;

//Queue for the data from the ADC
xQueueHandle sdQueue = NULL;

bool sd_mounted = false;

void sd_init() {
	ESP_LOGI(TAG, "Initializing SD card");

	gpio_configureSDCardDetect();

	sd_mounted = false;
	sd_csvFilePtr = NULL;
	sd_logFilePtr = NULL;

	if (gpio_SDCardDetected())
		sd_mount();
	
	//create queue and start task
	sdQueue = xQueueCreate(10, sizeof(int32_t));
	xTaskCreatePinnedToCore(&sd_task, "sd task", 1024, NULL, 4, NULL, 1);
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
	sd_openCSVFile();
	//sd_openLogFile();
	return ESP_OK;

}

void sd_unmount() {
	if (sd_mounted) {
		ESP_LOGI(TAG, "Unmounting SD card");
		sd_closeCSVFile();
		sd_closeLogFile();
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
	Opens a new CSV file
	Returns true if the file is opened
			false if the file doesn't exist or opening failed
*/
bool sd_openCSVFile() {
	if (!gpio_SDCardDetected() || !sd_mounted)
		return false;

	sd_closeCSVFile();
	int nr = 1;
	char path[32];
	do {
		memset(path, '\0', sizeof(path));
		sprintf(path, "%s(%d).csv", SD_CSV_FILENAME, nr);
		nr++;
	}
	while (fileIO_fileExists(SD_BASEPATH, path));

	sd_csvFilePtr = fileIO_openFile(SD_BASEPATH, path, "a");

	if (!sd_csvFilePtr) {
		ESP_LOGE(TAG, "Failed to open csv file: %s", path);
		return false;
	}
	return true;
}

/*
	Opens a new log file
	Returns true if the file is opened
			false if the file doesn't exist or opening failed
*/
bool sd_openLogFile() {
	if (!gpio_SDCardDetected() || !sd_mounted)
		return false;

	sd_closeLogFile();
	int nr = 1;
	char path[32];
	do {
		memset(path, '\0', sizeof(path));
		sprintf(path, "%s(%d).txt", SD_LOG_FILENAME, nr);
		nr++;
	}
	while (fileIO_fileExists(SD_BASEPATH, path));

	sd_logFilePtr = fileIO_openFile(SD_BASEPATH, path, "a");

	if (!sd_logFilePtr) {
		ESP_LOGE(TAG, "Failed to open log file: %s", path);
		return false;
	}
	return true;
}

/*
	Closes the opened file
*/
void sd_closeCSVFile() {
	if (!gpio_SDCardDetected() || !sd_mounted)
		return;

	fileIO_closeFile(sd_csvFilePtr);
	sd_csvFilePtr = NULL;
}

/*
	Closes the opened file
*/
void sd_closeLogFile() {
	if (!gpio_SDCardDetected() || !sd_mounted)
		return;

	fileIO_closeFile(sd_logFilePtr);
	sd_logFilePtr = NULL;
}

/*
	Append the given list of integers as CSV to a file
*/
void sd_appendCSV(int count, ...) {
	if (!gpio_SDCardDetected() || !sd_mounted)
		return;

	va_list list;
	va_start(list, count);
	char buf[SD_CSV_TEMP_BUF_LEN] = {'\0'};
	for (int i = 0; i < count; i++) {
		snprintf(buf, SD_CSV_TEMP_BUF_LEN - 2, "%d", va_arg(list, int)); // -2 for , or \n and \0
		if (i != (count-1))
			strcat(buf, ",");
		else
			strcat(buf, "\n");
	}
	fileIO_writeFile(sd_csvFilePtr, buf);
	va_end(list);
}

/*void sd_appendLog(char *log) {
	if (!gpio_SDCardDetected() || !sd_mounted)
		return;

	fileIO_writeFile(sd_logFilePtr, data);
}*/

/*
	Add a new value to the queue
*/
void sd_addToQueue(uint32_t val) {
	if (!xQueueSend(sdQueue, &val, 0))
		ESP_LOGE(TAG, "Failed to append value to queue");
}

/*
	SD card main task
*/
void sd_task(void *param) {
	uint32_t val;
	while (1) {
		if(xQueueReceive(sdQueue, &val, portMAX_DELAY)) {
			ESP_LOGI(TAG, "New value from ADC");

			//get latest values from algorithms
			int heartRate = alg_getHeartRate();
			int breathingRate = alg_getBreathingRate();

			sd_appendCSV(val, heartRate, breathingRate);
		}
	}
}
