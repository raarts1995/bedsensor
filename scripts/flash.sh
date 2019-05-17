#!/bin/bash
if [[ "$PWD" =~ scripts ]]; then
	cd ../
fi

echo "flashing ESP"
make -j3 flash

if [[ "$*" == "aws" ]] || [[ "$1" == "all" ]]; then
	echo "Configuring AWS"
	cd scripts/awsConfig/
	python configAws.py
	cd ../../
fi

if [[ "$*" == "spiffs" ]] || [[ "$1" == "all" ]]; then
	echo "Flashing SPIFFS content"
	bash scripts/uploadSpiffs.sh
fi

if [[ "$*" == "monitor" ]]; then
	echo "Opening monitor"
	make monitor
fi
