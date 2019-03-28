#ifndef SD_H
#define SD_H

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include "driver/sdspi_host.h"

#include "esp_err.h"
#include "esp_log.h"

#include "config.h"
#include "gpio.h"

#define SD_BASEPATH "/sdcard"

esp_err_t sd_init();

#endif