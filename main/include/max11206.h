#ifndef MAX11206_H
#define MAX11206_H

#include "esp_err.h"
#include "esp_log.h"

#include "spi.h"
#include "gpio.h"

#include "sd.h"
#include "algorithms.h"

#define ADC_MAX_CLOCK_FREQ 5000000 //5MHz

#define ADC_REG_STAT1 0x00 //8 bit:  status
#define ADC_REG_CTRL1 0x01 //8 bit:  control 1
#define ADC_REG_CTRL2 0x02 //8 bit:  control 2: GPIO direction and values
#define ADC_REG_CTRL3 0x03 //8 bit:  control 3: programmable gain and calibration settings
#define ADC_REG_DATA  0x04 //24 bit: data
#define ADC_REG_SOC   0x05 //24 bit: System Offset Calibration
#define ADC_REG_SGC   0x06 //24 bit: System Gain Calibration
#define ADC_REG_SCOC  0x07 //24 bit: Self-Cal Offset Calibration
#define ADC_REG_SCGC  0x08 //24 bit: Self-Cal Gain Calibration

#define ADC_CMD_SCC        0x10 //self calibration cycle
#define ADC_CMD_SOCC       0x20 //system offset calibration cycle
#define ADC_CMD_SGC        0x30 //system gain calibration
#define ADC_CMD_POWER_DOWN 0x08

#define ADC_REG_STAT1_RDY   (1 << 0)
#define ADC_REG_STAT1_MSTAT (1 << 1)
#define ADC_REG_STAT1_UR    (1 << 2)
#define ADC_REG_STAT1_OR    (1 << 3)
#define ADC_REG_STAT1_RATE_OFFSET 4

#define ADC_REG_CTRL1_SCYCLE (1 << 1)
#define ADC_REG_CTRL1_FORMAT (1 << 2)
#define ADC_REG_CTRL1_SIGBUF (1 << 3)
#define ADC_REG_CTRL1_REFBUF (1 << 4)
#define ADC_REG_CTRL1_EXTCLK (1 << 5)
#define ADC_REG_CTRL1_U_BN   (1 << 6)
#define ADC_REG_CTRL1_LINEF  (1 << 7)

#define ADC_REG_CTRL3_NOSCO  (1 << 1)
#define ADC_REG_CTRL3_NOSCG  (1 << 2)
#define ADC_REG_CTRL3_NOSYSO (1 << 3)
#define ADC_REG_CTRL3_NOSYSG (1 << 4)
#define ADC_REG_CTRL3_DGAIN_OFFSET 5

void adc_init();
int32_t adc_measure(uint8_t rate);
void adc_startConversion(uint8_t rate);
void adc_continuousMode(bool continuous);
bool adc_running();
bool adc_ready();
int32_t adc_readData();
void adc_selfCalibrate();

void adc_setInterrupt();
void adc_clearInterrupt();
void IRAM_ATTR adc_dataReadyIntr(void *params);
void adc_interruptHandlerTask(void* arg);

void adc_sendCommand(uint8_t command);
void adc_write8(uint8_t reg, uint8_t data);
uint8_t adc_read8(uint8_t reg);
void adc_write24(uint8_t reg, uint32_t data);
uint32_t adc_read24(uint8_t reg);
void adc_transfer(spiDevice spi, uint8_t *buffer, uint8_t len);

#endif