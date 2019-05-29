#include "algorithms.h"

#define TAG "Algorithms"

int heartRate;
int breathingRate;
int32_t hrLp;
int32_t hrPeak;
int32_t brLp;
int32_t brPeak;

//Queue for the data from the ADC
xQueueHandle algQueue = NULL;

//heart and breathing rate queues
xQueueHandle hrQueue = NULL;
xQueueHandle brQueue = NULL;

/*
	Initialize the algorithm task
*/
void alg_init() {
	heartRate = 0;
	breathingRate = 0;
	hrLp = hrPeak = brLp = brPeak = 0;

	//create queues and tasks
	algQueue = xQueueCreate(10, sizeof(int32_t));
	hrQueue = xQueueCreate(10, sizeof(int32_t));
	brQueue = xQueueCreate(10, sizeof(int32_t));
	xTaskCreatePinnedToCore(&alg_task, "algorithm task", 2048, NULL, 5, NULL, 1);
	xTaskCreatePinnedToCore(&alg_heartRateTask, "heart rate task", 2048, NULL, 4, NULL, 1);
	xTaskCreatePinnedToCore(&alg_breathingRateTask, "breathing rate task", 2048, NULL, 4, NULL, 1);
}

/*
	Function to be called when a person enters or leaves the bed
*/
void alg_personDetected(bool detected) {
	if (detected) { //person is in bed
		//connect to AWS and start AWS timer
	}
	else { //person left
		//stop AWS timer and disconnect from AWS
	}
}

/*
	Get the latest heartrate measurement
*/
int alg_getHeartRate() {
	return heartRate;
}

/*
	Get the latest breathing rate measurement
*/
int alg_getBreathingRate() {
	return breathingRate;
}

/*
	Add a new value to the queue
*/
void alg_addToQueue(uint32_t val) {
	if (!xQueueSend(algQueue, &val, 0))
		ESP_LOGE(TAG, "Failed to append value to queue");
}

/*
	Main algorithm task
*/
void alg_task(void *param) {
	int32_t val;
	
	while (1) {
		if(xQueueReceive(algQueue, &val, portMAX_DELAY)) {
			if (!xQueueSend(hrQueue, &val, 0))
				ESP_LOGE(TAG, "Failed to append to hrQueue");
			if (!xQueueSend(brQueue, &val, 0))
				ESP_LOGE(TAG, "Failed to append to brQueue");
			
			printf("%d %d %d %d %d %d %d\n", val, hrLp, hrPeak, heartRate, brLp, brPeak, breathingRate);
		}
	}
}

void alg_heartRateTask(void *param) {
	int32_t val;
	
	float bpParamB[] = {0.420808, 0       , -0.420808};
	float bpParamA[] = {1       , 0.841616, -0.158384};
	float lpParamB[] = {0.010432413371, 0.020864826742,  0.010432413371};
	float lpParamA[] = {1             , 1.690996376887, -0.732726030371};
	IIRFilter bp  = filter_IIRCreate(bpParamB, bpParamA);
	IIRFilter bp2 = filter_IIRCreate(bpParamB, bpParamA);
	IIRFilter lp  = filter_IIRCreate(lpParamB, lpParamA);
	IIRFilter lp2 = filter_IIRCreate(lpParamB, lpParamA);
	PeakFilter pf = filter_peakCreate();
	MedianFilter mf = filter_medianCreate();
	
	int prevPeak = 0;
	float hr = 0;
	
	while (1) {
		if(xQueueReceive(hrQueue, &val, portMAX_DELAY)) {
			//ESP_LOGI(TAG, "New value");
			
			//do algorithm things
			float bpResult = filter_IIRAdd(&bp2, filter_IIRAdd(&bp, (float)val));
			float energy   = bpResult * bpResult * log10(bpResult * bpResult);
			if (energy < 0) energy *= -1;
			float lpResult = filter_IIRAdd(&lp2, filter_IIRAdd(&lp, energy));
			PeakData pd = filter_peakDetect(&pf, lpResult);
			hrLp = (int32_t)lpResult;
			hrPeak = 0;
			if (pd.peakDetected) {
				hrPeak = 100;
				if ((prevPeak - pd.nrOfSamplesAgo) == 0)
					continue;
				hr = 3000 / (prevPeak - pd.nrOfSamplesAgo); //3000 = 60 (sec/min) * 50 Hz samplerate
				heartRate = filter_medianAdd(&mf, hr);
				prevPeak = pd.nrOfSamplesAgo;
			}
			
			prevPeak++;
			
			//printf("%f %d\n", hr, heartRate);
		}
	}
}

void alg_breathingRateTask(void *param) {
	int32_t val;
	float lpParamB[] = {0.000241359049, 0.000482718098,  0.000241359049};
	float lpParamA[] = {1             , 1.955578240315, -0.956543676511};
	IIRFilter lp  = filter_IIRCreate(lpParamB, lpParamA);
	IIRFilter lp2 = filter_IIRCreate(lpParamB, lpParamA);
	PeakFilter pf = filter_peakCreate();
	MedianFilter mf = filter_medianCreate();
	
	pf.backoffTimeThres = 50;
	pf.peakHeightThres = 50;
	
	int prevPeak = 0;
	float br = 0;
	
	while (1) {
		if(xQueueReceive(brQueue, &val, portMAX_DELAY)) {
			//ESP_LOGI(TAG, "New value");
			float lpResult = filter_IIRAdd(&lp2, filter_IIRAdd(&lp, (float)val));
			PeakData pd = filter_peakDetect(&pf, lpResult);
			brLp = (int32_t)lpResult;
			brPeak = 0;
			
			if (pd.peakDetected) {
				brPeak = 100;
				if ((prevPeak - pd.nrOfSamplesAgo) == 0)
					continue;
				br = 3000 / (prevPeak - pd.nrOfSamplesAgo); //3000 = 60 (sec/min) * 50 Hz samplerate
				breathingRate = filter_medianAdd(&mf, br);
				prevPeak = pd.nrOfSamplesAgo;
			}
			
			prevPeak++;
			
			//printf("%f %d\n", br, breathingRate);
		}
	}
}
