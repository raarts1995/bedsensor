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
	connectParams.clientIDLen = (uint16_t)strlen(/* lengte van bovenstaand id */);
	connectParams.isWillMsgPresent = false;

	IoT_Publish_Message_Params awsMsg;

	awsMsg.qos = QOS1; //QOS0: no ack, QOS1: ack
	awsMsg.payload = (void*)payload;
	awsMsg.isRetained = 0;

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

	//...
}

void aws_disconnectCallbackHandler() {

}
