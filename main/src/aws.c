#include "aws.h"

#define TAG "AWS"

AWS_IoT_Client aws_client;

/*
	Initialize the AWS IoT client
	SPIFFS should be initialized before this function is called
*/
bool aws_init() {
	IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;

	mqttInitParams.enableAutoReconnect = false;
	mqttInitParams.pHostURL = AWS_IOT_MQTT_HOST;
	mqttInitParams.port = AWS_IOT_MQTT_PORT;
	mqttInitParams.pRootCALocation = AWS_ROOT_CERT_PATH;
	mqttInitParams.pDeviceCertLocation = AWS_DEVICE_CERT_PATH;
	mqttInitParams.pDevicePrivateKeyLocation = AWS_PRIVATE_KEY_PATH;
	mqttInitParams.mqttCommandTimeout_ms = AWS_MQTT_COMMAND_TIMEOUT;
	mqttInitParams.tlsHandshakeTimeout_ms = AWS_TLS_HANDSHAKE_TIMEOUT;
	mqttInitParams.isSSLHostnameVerify = true;
	mqttInitParams.disconnectHandler = aws_disconnectCallbackHandler;
	mqttInitParams.disconnectHandlerData = NULL;

	IoT_Error_t iotErr = aws_iot_mqtt_init(&aws_client, &mqttInitParams);
	if (iotErr != SUCCESS) {
		ESP_LOGE(TAG, "Failed to initialize AWS (error: %d)", iotErr);
		return false;
	}

	ESP_LOGI(TAG, "AWS IoT SDK Version: %d.%d.%d-%s", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);
	return true;
}

bool aws_connect() {
	IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

	connectParams.keepAliveIntervalInSec = AWS_KEEP_ALIVE_INTERVAL;
	connectParams.isCleanSession = true;
	connectParams.MQTTVersion = MQTT_3_1_1;
	connectParams.pClientID = "ESPbedsensor"; //moet uniek zijn voor ieder apparaat
	connectParams.clientIDLen = (uint16_t)strlen(connectParams.pClientID);
	connectParams.isWillMsgPresent = false;

	IoT_Error_t iotErr = aws_iot_mqtt_connect(&aws_client, &connectParams);
	if (iotErr != SUCCESS) {
		ESP_LOGE(TAG, "Failed to connect to AWS (error: %d)", iotErr);
		return false;
	}
	ESP_LOGI(TAG, "AWS connected");
	return true;
}

void aws_disconnect() {
	aws_iot_mqtt_disconnect(&aws_client);
}

void aws_disconnectCallbackHandler(AWS_IoT_Client* client, void* data) {
	ESP_LOGI(TAG, "AWS disconnected");
	if (client == NULL)
		return;

	if (aws_iot_is_autoreconnect_enabled(client)) {
		ESP_LOGI(TAG, "Autoreconnect is enabled");
	}
	else {
		IoT_Error_t iotErr = aws_iot_mqtt_attempt_reconnect(client);
		if (iotErr == NETWORK_RECONNECTED) {
			ESP_LOGI(TAG, "Reconnected");
		}
		else {
			ESP_LOGE(TAG, "Failed to reconnect: %d", iotErr);
		}
	}
}

bool aws_sendData() {
	char* payload = (char*)malloc(AWS_MAX_PAYLOAD_SIZE*sizeof(char));

	IoT_Publish_Message_Params awsMsg;
	awsMsg.qos = QOS1; //QOS0: no ack, QOS1: ack
	awsMsg.payload = (void*)payload;
	awsMsg.isRetained = 0;

	//generate payload in json format
	sprintf(payload, 
		"{"
			"\"timestamp\":\"%u\","
			"\"chipID\":\"%s\","
			"\"heartrate\":\"%d\","
			"\"breathingrate\":\"%d\""
		"}",
	esp_random(), "chipID", 65, 20);
	awsMsg.payloadLen = strlen(payload);

	IoT_Error_t iotErr = aws_iot_mqtt_publish(&aws_client, AWS_TOPIC, strlen(AWS_TOPIC), &awsMsg);
	free(payload);
	
	if (iotErr != SUCCESS) {
		ESP_LOGE(TAG, "Failed to Publish message");
		return false;
	}
	return true;
}

void aws_testTask(void* param) {

	ESP_LOGI(TAG, "Starting AWS task");
	ESP_LOGI(TAG, "AWS_ROOT_CERT_PATH  : %s", AWS_ROOT_CERT_PATH   );
	ESP_LOGI(TAG, "AWS_DEVICE_CERT_PATH: %s", AWS_DEVICE_CERT_PATH );
	ESP_LOGI(TAG, "AWS_PRIVATE_KEY_PATH: %s", AWS_PRIVATE_KEY_PATH );

	ESP_LOGI(TAG, "Waiting for wifi connection");
	while (wifi_connectState() != WIFI_STATE_CONNECTED)
		vTaskDelay(100/portTICK_PERIOD_MS); //wacht 100ms
	ESP_LOGI(TAG, "Wifi connected");

	aws_init();
	while (1) {
		if (gpio_getButtonState()) {
			ESP_LOGI(TAG, "Button pressed");
			aws_connect();
			if (aws_sendData()) {
				ESP_LOGI(TAG, "Sending data succeeded");
			}
			else
				ESP_LOGE(TAG, "Sending data failed");
			aws_disconnect();

			//wait for button release
			while (gpio_getButtonState())
				vTaskDelay(100/portTICK_PERIOD_MS); //wacht 100ms
		}
		
		vTaskDelay(100/portTICK_PERIOD_MS); //wacht 100ms
	}
}
