#ifndef SD_H
#define SD_H

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include "driver/sdspi_host.h"
#include "driver/sdmmc_defs.h"

#include "esp_err.h"
#include "esp_log.h"

#include "espSystem.h"
#include "gpio.h"
#include "fileIO.h"

#define SD_BASEPATH "/sdcard"

void sd_init();
esp_err_t sd_mount();
void sd_unmount();
void sd_printCardInfo();
bool sd_fileExists(char* path);
bool sd_openFile(char* path);
void sd_appendFile(char* data);
void sd_closeFile();

#endif