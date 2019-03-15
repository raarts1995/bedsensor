# Bedsensor

Bedsensor project voor 2M Engineering. Het project is gemaakt voor de ESP32.

De code gebruikt de [ESP-IDF] omgeving voor de ESP32 op een Ubuntu 16.04.6 LTS OS.

## Compileren
Om de code te compileren is het script flash.sh gemaakt. Dit script: 
- compileert de code, gebruik makend van 3 threads (computer heeft 2 processors -> arg `-j3` volgens ESP-IDF)
- flasht de code naar de poort, ingesteld via `make menuconfig`
- opent de debug monitor

Start het script door `bash flash.sh` in een terminal venster te typen.

## SPIFFS
De webpagina is opgeslagen in het SPIFFS geheugen. Hiervoor is de took [mkspiffs] gebruikt.
Voor het flashen is een script gemaakt genaamd uploadSpiffs.sh. Dit script:
- maakt een binary van de inhoud van de map main/www
- flasht deze binary op de ESP32 in de SPIFFS partitie
- verwijdert de gemaakte binary

Start het script door `bash uploadSpiffs.sh` te typen in een terminal venster. 
De poort is standaard `/dev/ttyUSB0` maar kan aangepast worden door deze als argument mee te geven aan het script (vb.: `bash uploadSpiffs.sh /dev/ttyUSB0`).

Om het script te laten werken dient de [mkspiffs] tool in het systeem pad te staan.
Voeg daarom de regel `export MKSPIFFS_PATH=~/esp/mkspiffs` toe aan `~/.profile`

[ESP-IDF]: https://github.com/espressif/esp-idf
[mkspiffs]: https://github.com/igrr/mkspiffs
