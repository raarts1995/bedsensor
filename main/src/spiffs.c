#include "spiffs.h"
#define TAG "SPIFFS"

FILE* spiffs_filePtr;

/*
	Initializes the SPI Flash FileSystem
*/
esp_err_t spiffs_init(void) {
	ESP_LOGI(TAG, "Initializing SPIFFS");
	if (spiffs_mounted()) {
		ESP_LOGI(TAG, "SPIFFS already initialized");
		return ESP_OK;
	}

	spiffs_filePtr = NULL;

	esp_vfs_spiffs_conf_t conf = {
	  .base_path = SPIFFS_BASEPATH,
	  .partition_label = NULL,
	  .max_files = 5,   // This decides the maximum number of files that can be created on the storage
	  .format_if_mount_failed = true
	};

	esp_err_t ret = esp_vfs_spiffs_register(&conf);
	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		} else if (ret == ESP_ERR_NOT_FOUND) {
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		} else {
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
		}
		return ESP_FAIL;
	}

	size_t total = 0, used = 0;
	ret = esp_spiffs_info(NULL, &total, &used);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
		return ESP_FAIL;
	}

	ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
	return ESP_OK;
}

/*
	Deinitializes SPIFFS
*/
void spiffs_deinit() {
	esp_vfs_spiffs_unregister(NULL);
}

/*
	Checks if SPIFFS is mounted
	Returns true if it is
			false if it isn't
*/
bool spiffs_mounted() {
	bool mounted = esp_spiffs_mounted(NULL);
	if (!mounted)
		ESP_LOGI(TAG, "SPIFFS not mounted");
	return mounted;
}

/*
	Checks if the given file exists
*/
bool spiffs_fileExists(char* path) {
	if (!spiffs_mounted())
		return false;

	struct stat file_stat;
	char filepath[SPIFFS_MAX_FILENAME];

	/* Retrieve the base path of file storage to construct the full path*/
	strcpy(filepath, SPIFFS_BASEPATH);

	strcat(filepath, path);
	if (stat(filepath, &file_stat) == -1) {
		ESP_LOGE(TAG, "Failed to stat file : %s", filepath);
		return false;
	}
	return true;
}

/*
	Returns the size of a file
*/
size_t spiffs_getFileSize(char* path) {
	if (!spiffs_mounted())
		return 0;

	struct stat file_stat;
	char filepath[SPIFFS_MAX_FILENAME];

	/* Retrieve the base path of file storage to construct the full path*/
	strcpy(filepath, SPIFFS_BASEPATH);

	strcat(filepath, path);
	if (stat(filepath, &file_stat) == -1) {
		ESP_LOGE(TAG, "Failed to stat file : %s", filepath);
		return 0;
	}
	return file_stat.st_size;
}

/*
	Opens the requested file
	Returns true if the file is opened
			false if the file doesn't exist or opening failed
*/
bool spiffs_openFile(char* path) {
	if (!spiffs_mounted())
		return false;

	spiffs_closeFile();

	if (!spiffs_fileExists(path))
		return false;

	char filepath[SPIFFS_MAX_FILENAME];

	/* Retrieve the base path of file storage to construct the full path*/
	strcpy(filepath, SPIFFS_BASEPATH);
	if (path[0] != '/')
		strcat(filepath, "/");
	strcat(filepath, path);

	spiffs_filePtr = fopen(filepath, "r");
	if (!spiffs_filePtr) {
		ESP_LOGE(TAG, "SPIFFS: Failed to open file: %s", filepath);
		return false;
	}
	return true;
}

/*
	Reads the file and stores the content in the variable chunk
	Returns the amount of bytes read
*/
size_t spiffs_readFile(char* chunk, size_t chunkSize) {
	if (!spiffs_mounted())
		return 0;

	if (spiffs_filePtr)
		return fread(chunk, 1, chunkSize, spiffs_filePtr);
	return 0;
}

/*
	Closes the opened file
*/
void spiffs_closeFile() {
	if (!spiffs_mounted())
		return;

	if (spiffs_filePtr) {
		fclose(spiffs_filePtr);
		spiffs_filePtr = NULL;
	}
}
