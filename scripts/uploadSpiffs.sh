#!/bin/bash
if [[ "$PWD" =~ scripts ]]; then
	cd ../
fi

PORT="/dev/ttyUSB0"
BAUD=921600

#same as in partitions.csv
SPIFFSADDR=0x110000
SPIFFSSIZE=0x80000

if [ "$1" != "" ]; then
	PORT=$1
fi
echo "Port: $PORT"
echo "Baud: $BAUD"

if [ -d "main/www/" ]; then
	${MKSPIFFS_PATH}/mkspiffs -c main/www/ -b 4096 -p 256 -s $SPIFFSSIZE spiffs.bin
	python ${IDF_PATH}/components/esptool_py/esptool/esptool.py --chip esp32 --port $PORT --baud $BAUD write_flash -z $SPIFFSADDR spiffs.bin
	rm spiffs.bin
else
	echo "www folder doesn't exist"
fi
