deps_config := \
	/home/roy/esp/esp-idf/components/app_trace/Kconfig \
	/home/roy/esp/esp-idf/components/aws_iot/Kconfig \
	/home/roy/esp/esp-idf/components/bt/Kconfig \
	/home/roy/esp/esp-idf/components/driver/Kconfig \
	/home/roy/esp/esp-idf/components/efuse/Kconfig \
	/home/roy/esp/esp-idf/components/esp32/Kconfig \
	/home/roy/esp/esp-idf/components/esp_adc_cal/Kconfig \
	/home/roy/esp/esp-idf/components/esp_event/Kconfig \
	/home/roy/esp/esp-idf/components/esp_http_client/Kconfig \
	/home/roy/esp/esp-idf/components/esp_http_server/Kconfig \
	/home/roy/esp/esp-idf/components/esp_https_ota/Kconfig \
	/home/roy/esp/esp-idf/components/espcoredump/Kconfig \
	/home/roy/esp/esp-idf/components/ethernet/Kconfig \
	/home/roy/esp/esp-idf/components/fatfs/Kconfig \
	/home/roy/esp/esp-idf/components/freemodbus/Kconfig \
	/home/roy/esp/esp-idf/components/freertos/Kconfig \
	/home/roy/esp/esp-idf/components/heap/Kconfig \
	/home/roy/esp/esp-idf/components/libsodium/Kconfig \
	/home/roy/esp/esp-idf/components/log/Kconfig \
	/home/roy/esp/esp-idf/components/lwip/Kconfig \
	/home/roy/esp/esp-idf/components/mbedtls/Kconfig \
	/home/roy/esp/esp-idf/components/mdns/Kconfig \
	/home/roy/esp/esp-idf/components/mqtt/Kconfig \
	/home/roy/esp/esp-idf/components/nvs_flash/Kconfig \
	/home/roy/esp/esp-idf/components/openssl/Kconfig \
	/home/roy/esp/esp-idf/components/pthread/Kconfig \
	/home/roy/esp/esp-idf/components/spi_flash/Kconfig \
	/home/roy/esp/esp-idf/components/spiffs/Kconfig \
	/home/roy/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/home/roy/esp/esp-idf/components/unity/Kconfig \
	/home/roy/esp/esp-idf/components/vfs/Kconfig \
	/home/roy/esp/esp-idf/components/wear_levelling/Kconfig \
	/home/roy/esp/esp-idf/components/app_update/Kconfig.projbuild \
	/home/roy/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/roy/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/roy/esp/projects/bedsensor/main/Kconfig.projbuild \
	/home/roy/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/roy/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)

ifneq "$(IDF_TARGET)" "esp32"
include/config/auto.conf: FORCE
endif
ifneq "$(IDF_CMAKE)" "n"
include/config/auto.conf: FORCE
endif

$(deps_config): ;
