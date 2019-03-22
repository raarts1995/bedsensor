#include "aws.h"

#define TAG "AWS"

void aws_iot_task(void* param) {

	IoT_error_t iotErr = FAILURE;

	AWS_IoT_Client client;
	IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
	IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

	mqttInitParams.enableAutoReconnect = false;
	mqttInitParams.pHostUrl = AWS_IOT_MQTT_HOST;
	mqttInitParams.port = AWS_IOT_MQTT_PORT;
	mqttInitParams.pRootCALocation = AWS_ROOT_CERT_PATH;
	mqttInitParams.pDeviceCertLocation = AWS_DEVICE_CERT_PATH;
	mqttInitParams.pDevicePrivateKeyLocation = AWS_PRIVATE_KEY_PATH;
	mqttInitParams.mqttCommandTimeout_ms = 20000; //maak define
	mqttInitParams.tlsHandshakeTimeout_ms = 5000; //maak define
	mqttInitParams.isSSLHostnameVerify = true;
	mqttInitParams.disconnectHandler = aws_disconnectCallbackHandler;
	mqttInitParams.disconnectHandlerData = NULL;

	connectParams.keepAliveIntervalInSec = 10;
	connectParams.isCleanSession = true;
	connectParams.MQTTVersion = MQTT_3_1_1;
	connectParams.pClientID = /* uniek voor elk apparaat (chipID?) */
	connectParams.clientIDLen = (uint16_t)strlen(connectParams.pClientID);
	connectParams.isWillMsgPresent = false;

	iotErr = aws_iot_mqtt_init(&client, &mqttInitParams);
	if (iotErr != SUCCESS) {
		ESP_LOGE(TAG, "Failed to initialize AWS");
	}

	ESP_LOGI(TAG, "AWS IoT SDK Version: %d.%d.%d-%s", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

	char* payload = (char*)malloc(AWS_MAX_PAYLOAD_SIZE*sizeof(char));
	//generate payload in json format
	sprintf(payload, 
		"{"
			"\"timestamp\":\"%s\","
			"\"chipID\":\"%s\","
			"\"heartrate\":\"%d\","
			"\"breathingrate\":\"%d\""
		"}",
	"timestamp", "chipID", 65, 20);
	awsMsg.payloadLen = strlen(payload);

	iotErr = aws_iot_mqtt_connect(&client, &connectParams);
	if (iotErr != SUCCESS) {
		ESP_LOGE(TAG, "Failed to connect to AWS");
	}

	iotErr = aws_iot_mqtt_publish(&client, AWS_TOPIC, strlen(AWS_TOPIC), &awsMsg);
}

void aws_disconnectCallbackHandler(AWS_IoT_Client* client, void* data) {
	ESP_LOGI(TAG, "AWS disconnected");

}
