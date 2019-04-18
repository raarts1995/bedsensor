#include "max11206.h"
#define TAG "ADC"

#define ADC_WRITE_CMD(reg) (0xc0 | (reg << 1))        //set bit 7 and 8, clear bit 1: b110rrrr0
#define ADC_READ_CMD(reg) (0xc0 | (reg << 1) | 0x01) //set bit 7, 8 and 1: b110rrrr1

spiDevice adc;
bool adcRunning;

void adc_init() {
	//set adcRunning true while initializing adc
	adcRunning = true;
	adc = spi_addDevice(GPIO_ADC_CS, ADC_MAX_CLOCK_FREQ, 0);
	adc_write8(ADC_REG_CTRL1, ADC_REG_CTRL1_LINEF | ADC_REG_CTRL1_U_BN | ADC_REG_CTRL1_SCYCLE); //set 50 Hz filtering, unipolar, single conversion
	adc_selfCalibrate();
	adcRunning = false;
}

bool adc_running() {
	return adcRunning;
}

uint32_t adc_measure(uint8_t rate) {
	adcRunning = true;
	adc_sendCommand(rate & 0x07);
	while ((adc_read8(ADC_REG_STAT1) & ADC_REG_STAT1_RDY) == 0) {
		vTaskDelay(10/portTICK_PERIOD_MS);
	}
	uint32_t val = adc_read24(ADC_REG_DATA);
	val = (val >> 4) & 0xFFFFF;
	adcRunning = false;
	return val;
}

void adc_selfCalibrate() {

	//enable self calibration (set NOSCG and NOSCO to 0)
	adc_write8(ADC_REG_CTRL3, adc_read8(ADC_REG_CTRL3) & ~(ADC_REG_CTRL3_NOSCG | ADC_REG_CTRL3_NOSCO));
	adc_sendCommand(0x10); //start self calibration

	while (adc_read8(ADC_REG_STAT1) & ADC_REG_STAT1_MSTAT)
		vTaskDelay(100/portTICK_PERIOD_MS);

	if (adc_read8(ADC_REG_STAT1) & ADC_REG_STAT1_RDY)
		adc_read24(ADC_REG_DATA);
}

void adc_sendCommand(uint8_t command) {
	//set bit 8, clear bit 7: b10cccccc
	command = 0x80 | (command & ~(0xc0));
	spi_transfer(adc, &command, 1);
}

void adc_write8(uint8_t reg, uint8_t data) {
	uint8_t d[2] = {ADC_WRITE_CMD(reg), data};
	spi_transfer(adc, d, 2);
}

uint8_t adc_read8(uint8_t reg) {
	uint8_t d[2] = {ADC_READ_CMD(reg) | 0x01, 0};
	spi_transfer(adc, d, 2);
	return d[1];
}

void adc_write24(uint8_t reg, uint32_t data) {
	uint8_t d[4] = {ADC_WRITE_CMD(reg), data >> 16, data >> 8, data};
	spi_transfer(adc, d, 4);
}

uint32_t adc_read24(uint8_t reg) {
	uint8_t d[4] = {ADC_READ_CMD(reg), 0, 0, 0};
	//ESP_LOGI(TAG, "send: d[0]=%x, d[1]=%x, d[2]=%u, d[3]=%x", d[0],d[1],d[2],d[3]);
	spi_transfer(adc, d, 4);
	//ESP_LOGI(TAG, "recv: d[0]=%x, d[1]=%x, d[2]=%x, d[3]=%x", d[0],d[1],d[2],d[3]);
	return ((uint32_t)d[1] << 16) | ((uint32_t)d[2] << 8) | (uint32_t)d[3];
}