#include "max11206.h"
#define TAG "ADC"

#define ADC_WRITE_CMD(reg) (0xc0 | (reg << 1))        //set bit 7 and 8, clear bit 1: b110rrrr0
#define ADC_READ_CMD(reg) (0xc0 | (reg << 1) | 0x01) //set bit 7, 8 and 1: b110rrrr1

spiDevice adc;

//Queue for the data from the interrupt routine
xQueueHandle adcQueue = NULL;

/*
	Initialize the ADC driver
*/
void adc_init() {
	adc = spi_addDevice(SPI_PIN_NC, 400000, 0); //configure without CS pin
	gpio_configurePin(GPIO_ADC_CS, GPIO_MODE_INPUT_OUTPUT);
	gpio_setPin(GPIO_ADC_CS, 1); //strong independent ADC driver, sets CS pin itself
	
	adc_write8(ADC_REG_CTRL1, ADC_REG_CTRL1_LINEF); //set 50 Hz filtering
	adc_selfCalibrate();
	
	//create queue and start interrupt handler task
	adcQueue = xQueueCreate(10, sizeof(int32_t));
	xTaskCreate(adc_interruptHandlerTask, "adc_interruptHandlerTask", 2048, NULL, 10, NULL);
}

/*
	Start the AD conversion and return the value
*/
int32_t adc_measure(uint8_t rate) {
	adc_continuousMode(0);
	adc_startConversion(rate);
	while (!adc_ready()) {
		vTaskDelay(10/portTICK_PERIOD_MS);
	}
	int32_t val = adc_readData();
	return val;
}

/*
	Start the conversion
*/
void adc_startConversion(uint8_t rate) {
	adc_sendCommand(rate & 0x07);
}

/*
	Set the ADC in continuous mode or not
*/
void adc_continuousMode(bool continuous) {
	if (continuous)
		adc_write8(ADC_REG_CTRL1, adc_read8(ADC_REG_CTRL1) & ~(ADC_REG_CTRL1_SCYCLE)); //clear single cycle bit
	else
		adc_write8(ADC_REG_CTRL1, adc_read8(ADC_REG_CTRL1) | ADC_REG_CTRL1_SCYCLE); //set single cycle bit
}

/*
	Returns the state of the MSTAT bit
*/
bool adc_running() {
	return ((adc_read8(ADC_REG_STAT1) & ADC_REG_STAT1_MSTAT) != 0);
}

/*
	Returns the state of the RDY bit
*/
bool adc_ready() {
	return ((adc_read8(ADC_REG_STAT1) & ADC_REG_STAT1_RDY) != 0);
}

/*
	Read and parse the measured ADC value
*/
int32_t adc_readData() {
	int32_t val = (int32_t)adc_read24(ADC_REG_DATA);
	val = (val >> 4) & 0xFFFFF;
	if (val & (1 << 19))
		val |= 0xFFF00000; //convert 20 bit 2's complement to 32 bit
	return val;
}

/*
	Start the self calibration sequence
*/
void adc_selfCalibrate() {

	//enable self calibration (set NOSCG and NOSCO to 0)
	adc_write8(ADC_REG_CTRL3, adc_read8(ADC_REG_CTRL3) & ~(ADC_REG_CTRL3_NOSCG | ADC_REG_CTRL3_NOSCO));
	adc_sendCommand(ADC_CMD_SCC); //start self calibration

	while (adc_running())
		vTaskDelay(100/portTICK_PERIOD_MS);

	if (adc_ready())
		adc_read24(ADC_REG_DATA);
}

/*
	Attach an interrupt to the MISO line
	Used in continuous mode to detect a finished conversion
*/
void adc_setInterrupt() {
	gpio_attachInterrupt(GPIO_MISO, GPIO_PIN_INTR_POSEDGE, adc_dataReadyIntr);
	gpio_setPin(GPIO_ADC_CS, 0);
}

/*
	Detach the interrupt
*/
void adc_clearInterrupt() {
	gpio_setPin(GPIO_ADC_CS, 1);
	gpio_detachInterrupt(GPIO_MISO);
}

/*
	Data ready interupt routine
*/
void IRAM_ATTR adc_dataReadyIntr(void *params) {
	adc_clearInterrupt();
	int32_t val = adc_readData();
	adc_setInterrupt();
	xQueueSendFromISR(adcQueue, &val, 0);
}

/*
	Task which handles the data placed in the queue by the interrupt routine
*/
void adc_interruptHandlerTask(void* arg) {
	int32_t val;
	for(;;) {
		if(xQueueReceive(adcQueue, &val, portMAX_DELAY)) {
			printf("%d\n", val);
		}
	}
}

void adc_sendCommand(uint8_t command) {
	//set bit 8, clear bit 7: b10cccccc
	command = 0x80 | (command & ~(0xC0));
	adc_transfer(adc, &command, 1);
}

void adc_write8(uint8_t reg, uint8_t data) {
	uint8_t d[2] = {ADC_WRITE_CMD(reg), data};
	adc_transfer(adc, d, 2);
}

uint8_t adc_read8(uint8_t reg) {
	uint8_t d[2] = {ADC_READ_CMD(reg) | 0x01, 0};
	adc_transfer(adc, d, 2);
	return d[1];
}

void adc_write24(uint8_t reg, uint32_t data) {
	uint8_t d[4] = {ADC_WRITE_CMD(reg), data >> 16, data >> 8, data};
	adc_transfer(adc, d, 4);
}

uint32_t adc_read24(uint8_t reg) {
	uint8_t d[4] = {ADC_READ_CMD(reg), 0, 0, 0};
	//ESP_LOGI(TAG, "send: d[0]=%x, d[1]=%x, d[2]=%u, d[3]=%x", d[0],d[1],d[2],d[3]);
	adc_transfer(adc, d, 4);
	//ESP_LOGI(TAG, "recv: d[0]=%x, d[1]=%x, d[2]=%x, d[3]=%x", d[0],d[1],d[2],d[3]);
	return ((uint32_t)d[1] << 16) | ((uint32_t)d[2] << 8) | (uint32_t)d[3];
}

void adc_transfer(spiDevice spi, uint8_t *buffer, uint8_t len) {
	gpio_setPin(GPIO_ADC_CS, 0);
	spi_transfer(spi, buffer, len);
	gpio_setPin(GPIO_ADC_CS, 1);
}
