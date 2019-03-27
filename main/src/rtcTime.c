#include "rtcTime.h"
#define TAG "RTC"

void rtcTime_init() {
	ESP_LOGI(TAG, "Initializing RTC and SNTP protocol");

	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, "pool.ntp.org");
	sntp_init();
}

/*
	Returns the amount of seconds since 1-1-1970
*/
time_t rtcTime_getTime() {
	time_t t = 0;
	time(&t); //returns seconds since 1970
	return t;
}

/*
	Checks if the received time is valid
	Returns false if the year is 1970, true otherwise
*/
bool rtcTime_timeValid() {
	time_t t = rtcTime_getTime();
	struct tm timeInfo = {0};
	localtime_r(&t, &timeInfo);
	if ((timeInfo.tm_year + 1900) != 1970) {
		ESP_LOGI(TAG, "Year is not 1970. Time is probably valid (y: %d)", timeInfo.tm_year+1900);
		return true;
	}
	else {
		ESP_LOGI(TAG, "Year is 1970. Time not valid");
		return false;
	}
}

/*
	Gets the time values of the current time
*/
void rtcTime_getDate(int* day, int* month, int* year, int* hour, int* min, int* sec) {
	time_t t = rtcTime_getTime();
	struct tm timeInfo = {0};
	localtime_r(&t, &timeInfo);
	if (day != NULL)
		*day = timeInfo.tm_mday;
	if (month != NULL)
		*month = timeInfo.tm_mon+1;
	if (year != NULL)
		*year = timeInfo.tm_year + 1900;
	if (hour != NULL)
		*hour = timeInfo.tm_hour;
	if (min != NULL)
		*min = timeInfo.tm_min;
	if (sec != NULL)
		*sec = timeInfo.tm_sec;
}

/*
	Get the current time in string format
*/
void rtcTime_getDateString(char* dest, char* template, size_t maxLen) {
	time_t t = rtcTime_getTime();
	struct tm timeInfo = {0};
	localtime_r(&t, &timeInfo);
	strftime(dest, maxLen, template, &timeInfo);
}
