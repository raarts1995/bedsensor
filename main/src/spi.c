#include "spi.h"
#define TAG "SPI"

void spi_init() {
	ESP_LOGI(TAG, "Initializing SPI driver");
	spi_bus_config_t buscfg = {
		.miso_io_num = GPIO_MISO,
		.mosi_io_num = GPIO_MOSI,
		.sclk_io_num = GPIO_SCK,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = SPI_MAX_TRANSFER_SIZE
	};
	ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &buscfg, 0));
}

spiDevice spi_addDevice(int csPin, uint32_t speed, int mode) {
	ESP_LOGI(TAG, "Adding device (cs: %d, speed: %u kHz, mode: %d)", csPin, speed/1000, mode);
	spi_device_interface_config_t dev = {0};
	dev.clock_speed_hz = speed;
	dev.mode = mode;
	dev.spics_io_num = csPin;
	dev.queue_size = 1;

	spiDevice handle;
	ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &dev, &handle));
	return handle;
}

void spi_transfer(spiDevice spi, uint8_t *buffer, uint8_t len) {
	uint8_t *tmp = (uint8_t*)malloc(len*sizeof(uint8_t));
	if (len > SPI_MAX_TRANSFER_SIZE)
		len = SPI_MAX_TRANSFER_SIZE;
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));
	t.length = len * 8; //in bits
	t.tx_buffer = buffer;
	t.rx_buffer = tmp;

	ESP_ERROR_CHECK(spi_device_acquire_bus(spi, portMAX_DELAY));
	ESP_ERROR_CHECK(spi_device_polling_transmit(spi, &t));
	spi_device_release_bus(spi);
	memcpy(buffer, tmp, len);
	free(tmp);
}