#ifndef RTCTIME_H
#define RTCTIME_H

#include <time.h>
#include <sys/time.h>

#include "lwip/apps/sntp.h"

#include "esp_err.h"
#include "esp_log.h"

#include "wifi.h"

void rtcTime_init();
time_t rtcTime_getTime();
bool rtcTime_timeValid();
void rtcTime_getDate(int* day, int* month, int* year, int* hour, int* min, int* sec);
void rtcTime_getDateString(char* dest, char* template, size_t maxLen);

#endif