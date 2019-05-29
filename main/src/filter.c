#include "filter.h"

#define TAG "Filter"

IIRFilter filter_IIRCreate(float *b, float *a)  {
	IIRFilter f;
	for (int i = 0; i < IIRFILTER_COEF; i++) {
		f.b[i] = b[i];
		f.a[i] = a[i];
		f.xVal[i] = 0;
		f.yVal[i] = 0;
	}
	return f;
}

float filter_IIRAdd(IIRFilter *f, float sample) {
	for (int i = IIRFILTER_COEF - 1; i > 0; i--) {
		f->xVal[i] = f->xVal[i - 1];
		f->yVal[i] = f->yVal[i - 1];
	}
	f->xVal[0] = sample;
	f->yVal[0] = 0;
	
	//perform filter calculation
	for (int i = 0; i < IIRFILTER_COEF; i++) {
		f->yVal[0] += f->xVal[i]*f->b[i];
		if (i != 0)
			f->yVal[0] += f->yVal[i]*f->a[i];
	}
	return f->yVal[0];
}

PeakFilter filter_peakCreate() {
	PeakFilter pf = {false, 1000, 15, 0, 0, 0, FLT_MIN, FLT_MAX, 0, 0};
	return pf;
}

PeakData filter_peakDetect(PeakFilter *pf, float sample) {
	PeakData pd = {false, 0, 0};
	if (pf->detectValleys) sample *= -1;
	
	pf->prevSample = pf->currentSample;
	pf->currentSample = pf->nextSample;
	pf->nextSample = sample;
	
	if (pf->possiblePeakIndex < 0xFFFFFFFF) pf->possiblePeakIndex++;
	if (pf->previousPeakIndex < 0xFFFFFFFF) pf->previousPeakIndex++;
	
	//detect local peaks
	if (pf->prevSample <= pf->currentSample && pf->nextSample <= pf->currentSample) { //possible peak detected
		if (pf->currentSample > pf->peakValue &&
			pf->currentSample > (pf->valleyValue + pf->peakHeightThres) &&
			pf->previousPeakIndex > pf->backoffTimeThres) {
				pf->peakValue = pf->currentSample;
				pf->possiblePeakIndex = 1;
		}
	}
	
	//detect local minimum
	if (pf->prevSample >= pf->currentSample && pf->nextSample >= pf->currentSample) {
		//peak confirmed
		if (pf->peakValue > (pf->currentSample + pf->peakHeightThres)) {
			pd.peakDetected = true;
			pd.nrOfSamplesAgo = pf->possiblePeakIndex;
			pd.sampleValue = pf->peakValue;
			if (pf->detectValleys) pd.sampleValue *= -1;
			pf->previousPeakIndex = pf->possiblePeakIndex;
			pf->peakValue = FLT_MIN;
			pf->valleyValue = FLT_MAX;
		}
		if (pf->currentSample <= pf->valleyValue)
			pf->valleyValue = pf->currentSample;
	}
	
	return pd;
}

MedianFilter filter_medianCreate() {
	MedianFilter mf;
	for (int i = 0; i < MEDIANFILTER_SIZE; i++) {
		if (i < (MEDIANFILTER_SIZE / 2)) mf.buf[i] = FLT_MIN;
		else mf.buf[i] = FLT_MAX;
	}
	return mf;
}

float filter_medianAdd(MedianFilter *mf, float sample) {
	for (int i = MEDIANFILTER_SIZE - 1; i > 0; i--)
		mf->buf[i] = mf->buf[i - 1];
	mf->buf[0] = sample;
	
	int samplesBigger[MEDIANFILTER_SIZE] = {0};
	for (int i = 0; i < MEDIANFILTER_SIZE; i++) {
		samplesBigger[i] = 0;
		for (int j = 0; j < MEDIANFILTER_SIZE; j++) {
			if (mf->buf[j] >= mf->buf[i]) samplesBigger[i]++;
		}
		if (samplesBigger[i] == 3) return mf->buf[i];
	}
	for (int i = 0; i < MEDIANFILTER_SIZE; i++) {
		if (samplesBigger[i] == 4) return mf->buf[i];
	}
	for (int i = 0; i < MEDIANFILTER_SIZE; i++) {
		if (samplesBigger[i] == 5) return mf->buf[i];
	}
	return mf->buf[0];
}
