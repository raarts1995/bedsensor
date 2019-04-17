#ifndef AWS_H
#define AWS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include "esp_err.h"
#include "esp_log.h"

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"

#include "wifi.h"
#include "gpio.h"
#include "spiffs.h"
#include "rtcTime.h"
#include "espSystem.h"

#define AWS_ROOT_CERT   "/aws_root_cert.pem"
#define AWS_DEVICE_CERT "/aws_device_cert.pem.crt"
#define AWS_PRIVATE_KEY "/aws_private_key.pem.key"

#define AWS_ROOT_CERT_PATH   SPIFFS_BASEPATH AWS_ROOT_CERT
#define AWS_DEVICE_CERT_PATH SPIFFS_BASEPATH AWS_DEVICE_CERT
#define AWS_PRIVATE_KEY_PATH SPIFFS_BASEPATH AWS_PRIVATE_KEY

#define AWS_MQTT_COMMAND_TIMEOUT 20000 //ms
#define AWS_TLS_HANDSHAKE_TIMEOUT 5000 //ms
#define AWS_KEEP_ALIVE_INTERVAL 10 //seconds

#define AWS_MAX_PAYLOAD_SIZE 1024
#define AWS_TOPIC "bedsensor/measurements"

#define AWS_UPLOAD_INTERVAL 5000 //ms

bool aws_init();
void aws_timerTask(TimerHandle_t tmr);
bool aws_connect();
void aws_disconnect();
void aws_disconnectCallbackHandler(AWS_IoT_Client* client, void* data);
bool aws_sendData();
char* aws_constructPayload();
void aws_testTask(void* param);

#endif
