#ifndef AWS_H
#define AWS_H

#include "esp_err.h"
#include "esp_log.h"

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"

#include "spiffs.h"
#include "config.h"

#define AWS_ROOT_CERT "aws_root_cert.pem"
#define AWS_DEVICE_CERT "aws_device_cert.pem.crt"
#define AWS_PRIVATE_KEY "aws_private_key.pem.key"

#define AWS_ROOT_CERT_PATH SPIFFS_BASEPATH AWS_ROOT_CERT
#define AWS_DEVICE_CERT_PATH SPIFFS_BASEPATH AWS_DEVICE_CERT
#define AWS_PRIVATE_KEY_PATH SPIFFS_BASEPATH AWS_PRIVATE_KEY

#define AWS_MAX_PAYLOAD_SIZE 1024
#define AWS_TOPIC "bedsensor/measurements"

#endif