deps_config := \
	/opt/esp-idf/components/app_trace/Kconfig \
	/opt/esp-idf/components/aws_iot/Kconfig \
	/opt/esp-idf/components/bt/Kconfig \
	/opt/esp-idf/components/driver/Kconfig \
	/opt/esp-idf/components/efuse/Kconfig \
	/opt/esp-idf/components/esp32/Kconfig \
	/opt/esp-idf/components/esp_adc_cal/Kconfig \
	/opt/esp-idf/components/esp_event/Kconfig \
	/opt/esp-idf/components/esp_http_client/Kconfig \
	/opt/esp-idf/components/esp_http_server/Kconfig \
	/opt/esp-idf/components/esp_https_ota/Kconfig \
	/opt/esp-idf/components/espcoredump/Kconfig \
	/opt/esp-idf/components/ethernet/Kconfig \
	/opt/esp-idf/components/fatfs/Kconfig \
	/opt/esp-idf/components/freemodbus/Kconfig \
	/opt/esp-idf/components/freertos/Kconfig \
	/opt/esp-idf/components/heap/Kconfig \
	/home/nicooo/Code/esp32/apps/weather_station/components/i2cdev/Kconfig \
	/opt/esp-idf/components/libsodium/Kconfig \
	/opt/esp-idf/components/log/Kconfig \
	/opt/esp-idf/components/lwip/Kconfig \
	/opt/esp-idf/components/mbedtls/Kconfig \
	/opt/esp-idf/components/mdns/Kconfig \
	/opt/esp-idf/components/mqtt/Kconfig \
	/opt/esp-idf/components/nvs_flash/Kconfig \
	/opt/esp-idf/components/openssl/Kconfig \
	/opt/esp-idf/components/pthread/Kconfig \
	/opt/esp-idf/components/spi_flash/Kconfig \
	/opt/esp-idf/components/spiffs/Kconfig \
	/opt/esp-idf/components/tcpip_adapter/Kconfig \
	/opt/esp-idf/components/unity/Kconfig \
	/opt/esp-idf/components/vfs/Kconfig \
	/opt/esp-idf/components/wear_levelling/Kconfig \
	/opt/esp-idf/components/wifi_provisioning/Kconfig \
	/opt/esp-idf/components/app_update/Kconfig.projbuild \
	/opt/esp-idf/components/bootloader/Kconfig.projbuild \
	/opt/esp-idf/components/esptool_py/Kconfig.projbuild \
	/opt/esp-idf/components/partition_table/Kconfig.projbuild \
	/opt/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)

ifneq "$(IDF_TARGET)" "esp32"
include/config/auto.conf: FORCE
endif
ifneq "$(IDF_CMAKE)" "n"
include/config/auto.conf: FORCE
endif

$(deps_config): ;
