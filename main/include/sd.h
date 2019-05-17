#ifndef SD_H
#define SD_H

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include "driver/sdspi_host.h"
#include "driver/sdmmc_defs.h"

#include "esp_err.h"
#include "esp_log.h"

#include "espSystem.h"
#include "gpio.h"
#include "fileIO.h"
#include "algorithms.h"

#define SD_BASEPATH "/sdcard"

#define SD_CSV_TEMP_BUF_LEN 128

void sd_init();
esp_err_t sd_mount();
void sd_unmount();
void sd_printCardInfo();
bool sd_fileExists(char* path);
bool sd_openFile(char* path);
void sd_appendFile(char* data);
void sd_closeFile();
void sd_appendCSV(int count, ...);
void sd_addToQueue(uint32_t val);
void sd_task(void *param);

#endif