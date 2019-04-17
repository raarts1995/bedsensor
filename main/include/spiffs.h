#ifndef  SPIFFS_H
#define  SPIFFS_H

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/unistd.h>

#include "esp_vfs.h"
#include "esp_spiffs.h"

#include "esp_err.h"
#include "esp_log.h"

#include "fileIO.h"
#include "espSystem.h"

#define SPIFFS_BASEPATH "/spiffs"

//Max length a file path can have on storage
#define SPIFFS_MAX_FILENAME (ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN)

esp_err_t spiffs_init();
void spiffs_deinit();
bool spiffs_mounted();
bool spiffs_fileExists(char* path);
size_t spiffs_getFileSize(char* path);
bool spiffs_openFile(char* path);
size_t spiffs_readFile(char* chunk, size_t chunkSize);
void spiffs_closeFile();

#endif