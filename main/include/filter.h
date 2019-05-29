#ifndef FILTER_H
#define FILTER_H

#include <float.h>
#include <math.h>

#include "esp_err.h"
#include "esp_log.h"

#include "espSystem.h"
#include "filter.h"

#define IIRFILTER_COEF 3
#define MEDIANFILTER_SIZE 5

typedef struct {
	float a[IIRFILTER_COEF];
	float b[IIRFILTER_COEF];

	float xVal[IIRFILTER_COEF];
	float yVal[IIRFILTER_COEF];
} IIRFilter;

typedef struct {
	bool detectValleys;
	float peakHeightThres;
	int backoffTimeThres;
	float prevSample;
	float currentSample;
	float nextSample;
	float peakValue;
	float valleyValue;
	uint32_t possiblePeakIndex;
	uint32_t previousPeakIndex;
}	PeakFilter;

typedef struct {
	bool peakDetected;
	int nrOfSamplesAgo;
	float sampleValue;
} PeakData;

typedef struct {
	float buf[MEDIANFILTER_SIZE];
} MedianFilter;

IIRFilter filter_IIRCreate(float *b, float *a);
float filter_IIRAdd(IIRFilter *f, float sample);
PeakFilter filter_peakCreate();
PeakData filter_peakDetect(PeakFilter *pf, float sample);
MedianFilter filter_medianCreate();
float filter_medianAdd(MedianFilter *mf, float sample);

#endif