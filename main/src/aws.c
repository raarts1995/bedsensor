#include "aws.h"

#define TAG "AWS"

AWS_IoT_Client aws_client;

TimerHandle_t awsTimer;
bool aws_timerBusy;

/*
	Initialize the AWS IoT client and the upload timer
	SPIFFS should be initialized before this function is called
*/
bool aws_init() {
	aws_timerBusy = false;
	awsTimer = xTimerCreate(
		"aws timer", //timer name
		AWS_UPLOAD_INTERVAL/portTICK_PERIOD_MS, //timer period
		pdTRUE, //autoreload
		NULL, //timer ID
		aws_timerTask //callback function
	);

	if (!spiffs_fileExists(AWS_ROOT_CERT) || !spiffs_fileExists(AWS_DEVICE_CERT) || !spiffs_fileExists(AWS_PRIVATE_KEY)) {
		ESP_LOGE(TAG, "AWS certificates unknown");
		return false;
	}

	IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;

	mqttInitParams.enableAutoReconnect = true;
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

	xTaskCreatePinnedToCore(&aws_testTask, "aws test task", 4096, NULL, 3, NULL, 1);

	return true;
}

/*
	AWS timer task
	Constructs a payload and sends this to AWS
*/
void aws_timerTask(TimerHandle_t tmr) {
	if (aws_timerBusy) {
		ESP_LOGE(TAG, "Timer busy...");
		return;
	}
	aws_timerBusy = true;
	if (rtcTime_timeValid()) {
		if (!aws_sendData()) {
			ESP_LOGI(TAG, "Attempt reconnect and retry");
			aws_disconnect();
			if (aws_connect())
				if (aws_sendData())
					ESP_LOGI(TAG, "Data sent");
		}
		else
			ESP_LOGI(TAG, "Data sent");
	}
	aws_timerBusy = false;
}

/*
	Connect to AWS
*/
bool aws_connect() {
	if (wifi_connectState() != WIFI_STATE_CONNECTED) {
		ESP_LOGE(TAG, "Wifi not connected");
		return false;
	}

	IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

	connectParams.keepAliveIntervalInSec = AWS_KEEP_ALIVE_INTERVAL;
	connectParams.isCleanSession = true;
	connectParams.MQTTVersion = MQTT_3_1_1;
	connectParams.pClientID = espSystem_getMacAddr(); //moet uniek zijn voor ieder apparaat (MAC address)
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

/*
	Disconnect from AWS
*/
void aws_disconnect() {
	aws_iot_mqtt_disconnect(&aws_client);
}

/*
	AWS disconnect callback handler
	Gets called by the AWS API when it disconnects from AWS
*/
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

/*
	Send the measurment values to AWS
*/
bool aws_sendData() {
	char* payload = aws_constructPayload();
	if (payload == NULL)
		return false;

	ESP_LOGI(TAG, "Payload: %s", payload);

	IoT_Publish_Message_Params awsMsg;
	awsMsg.qos = QOS1; //QOS0: no ack, QOS1: ack
	awsMsg.payload = (void*)payload;
	awsMsg.payloadLen = strlen(payload);
	awsMsg.isRetained = 0;

	IoT_Error_t iotErr = aws_iot_mqtt_publish(&aws_client, AWS_TOPIC, strlen(AWS_TOPIC), &awsMsg);
	free(payload);
	
	if (iotErr != SUCCESS) {
		ESP_LOGE(TAG, "Failed to Publish message");
		return false;
	}
	return true;
}

/*
	Construct the payload to be sent to AWS
*/
char* aws_constructPayload() {
	char* payload = (char*)malloc(AWS_MAX_PAYLOAD_SIZE*sizeof(char));

	//generate payload in json format
	sprintf(payload, 
		"{"
			"\"chipID\":\"%s\","
			"\"time\":%lu,"
			"\"heartrate\":%d,"
			"\"breathingrate\":%d"
		"}",
		espSystem_getMacAddr(), 
		rtcTime_getTime(), 
		alg_getHeartRate(), 
		alg_getBreathingRate()
	);
	return payload;
}

/*
	Start the AWS upload timer
*/
void aws_startTimer() {
	if (!aws_timerRunning()) {
		aws_connect();
		if (!xTimerStart(awsTimer, 0)) {
			ESP_LOGE(TAG, "Failed to start timer");
		}
		else {
			ESP_LOGI(TAG, "Timer started");
		}
	}
}

/*
	Stop the AWS upload timer
*/
void aws_stopTimer() {
	if (!xTimerStop(awsTimer, 0)) {
		ESP_LOGE(TAG, "Failed to stop timer");
	}
	else {
		ESP_LOGI(TAG, "Timer stopped");
	}
	aws_disconnect();
}

/*
	Get the running state of the timer
	Returns true if the timer is running
			false if not
*/
bool aws_timerRunning() {
	return xTimerIsTimerActive(awsTimer);
}

/*
	Test task to test the AWS functionality
*/
void aws_testTask(void* param) {

	ESP_LOGI(TAG, "Starting AWS task");
	ESP_LOGI(TAG, "AWS_ROOT_CERT_PATH  : %s", AWS_ROOT_CERT_PATH   );
	ESP_LOGI(TAG, "AWS_DEVICE_CERT_PATH: %s", AWS_DEVICE_CERT_PATH );
	ESP_LOGI(TAG, "AWS_PRIVATE_KEY_PATH: %s", AWS_PRIVATE_KEY_PATH );

	ESP_LOGI(TAG, "Waiting for wifi connection");
	while (wifi_connectState() != WIFI_STATE_CONNECTED)
		vTaskDelay(100/portTICK_PERIOD_MS); //wacht 100ms
	ESP_LOGI(TAG, "Wifi connected");

	aws_connect();

	while (1) {
		if (gpio_getButtonState()) {
			ESP_LOGI(TAG, "Button pressed");
			if (aws_timerRunning()) {
				ESP_LOGI(TAG, "timer is running -> stopping timer");
				aws_stopTimer();
			}
			else {
				ESP_LOGI(TAG, "timer is not running -> starting timer");
				aws_startTimer();
			}
			//aws_timerTask(NULL);

			//wait for button release
			while (gpio_getButtonState())
				vTaskDelay(100/portTICK_PERIOD_MS); //wacht 100ms
		}
		
		vTaskDelay(100/portTICK_PERIOD_MS); //wacht 100ms
	}
}