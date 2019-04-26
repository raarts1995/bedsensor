#ifndef SPI_H
#define SPI_H

#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"

#include "espSystem.h"
#include "gpio.h"

#define SPI_MAX_TRANSFER_SIZE 32 //max size without using DMA transfer

#define SPI_PIN_NC -1 //not connected

typedef spi_device_handle_t spiDevice;

void spi_init();
spiDevice spi_addDevice(int csPin, uint32_t speed, int mode);
void spi_transfer(spiDevice spi, uint8_t *buffer, uint8_t len);

#endif