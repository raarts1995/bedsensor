if [[ "$PWD" =~ scripts ]]; then
	cd ../
fi
python ${IDF_PATH}/components/partition_table/gen_esp32part.py partitions.csv parts.bin
python ${IDF_PATH}/components/partition_table/gen_esp32part.py parts.bin
rm parts.bin
