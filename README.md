# Bedsensor

Bedsensor project voor 2M Engineering. Het project is gemaakt voor de ESP32.

De code gebruikt de [ESP-IDF] omgeving voor de ESP32 op een Ubuntu 18.04.6 LTS OS.

## Compileren
Om de code te compileren is het script flash.sh gemaakt. Dit script: 
- compileert de code, gebruik makend van 3 threads (computer heeft 2 processors -> arg `-j3` volgens ESP-IDF)
- flasht de code naar de poort, ingesteld via `make menuconfig`

Argumenten:
`aws`     Configureerd het AWS platform, gebruik makend van het MAC adres van de module. Ook worden de benodigde certificaten opgeslagen in `main/www`.
`spiffs`  Flasht de inhoud van de map `main/www` naar het SPIFFS geheugen.
`all`     Doet bovenstaande stappen in een keer
`monitor` Start de seriële monitor na het flashen.

Start het script door `bash scripts/flash.sh [args]` in een terminal venster te typen.

## SPIFFS
De webpagina is opgeslagen in het SPIFFS geheugen. Hiervoor is de took [mkspiffs] gebruikt.
Voor het flashen is een script gemaakt genaamd uploadSpiffs.sh. Dit script:
- maakt een binary van de inhoud van de map main/www
- flasht deze binary op de ESP32 in de SPIFFS partitie
- verwijdert de gemaakte binary

Start het script door `bash scripts/uploadSpiffs.sh` te typen in een terminal venster. 
De poort is standaard `/dev/ttyUSB0` maar kan aangepast worden door deze als argument mee te geven aan het script (vb.: `bash scripts/uploadSpiffs.sh /dev/ttyUSB0`).

Om het script te laten werken dient de [mkspiffs] tool in het systeem pad te staan.
Voeg daarom de regel `export MKSPIFFS_PATH=~/esp/mkspiffs` toe aan `~/.profile`

## AWS
Het project maak gebruikt van Amazon AWS. Om dit automatisch te configureren of te verwijderen zijn twee python scripts gemaakt, gebruik makend van de AWS SDK [boto3].
Het configuratiescript maakt een nieuw apparaat aan en genereerd de benodigde certificaten.
Voor het configureren dient het apparaat aangesloten te zijn, aangezien dan automatisch het MAC adres gelezen wordt.
Het configuratiescript kan gestart worden met het commando: `python configAws.py`

[ESP-IDF]: https://github.com/espressif/esp-idf
[mkspiffs]: https://github.com/igrr/mkspiffs
[boto3]: https://github.com/boto/boto3
