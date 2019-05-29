#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_err.h"
#include "esp_log.h"

#include "espSystem.h"
#include "filter.h"

void alg_init();
void alg_personDetected(bool detected);
int alg_getHeartRate();
int alg_getBreathingRate();
void alg_addToQueue(uint32_t val);
void alg_task(void *param);
void alg_heartRateTask(void *param);
void alg_breathingRateTask(void *param);

#endif