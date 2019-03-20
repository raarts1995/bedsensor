#include "server.h"

TimerHandle_t server_timeoutTimer = NULL;

httpd_handle_t server = NULL;

/*
	Function which gets called when the timeout timer expires
	Tries to connect to saved wifi network
*/
void server_timeoutFunction(TimerHandle_t tmr) {
	ESP_LOGI(TAG, "AP 10 min without activity");
	if (!wifi_getWifiSettings()) {
		ESP_LOGI(TAG, "No settings stored.  Keeping AP active");
		server_resetTimer();
		return;
	}

	//retry to connect to sta
	wifi_connectSTA();
}

/*
	Starts the server timeout timer
	Should only be called when the server starts
*/
void server_startTimer() {
	server_timeoutTimer = xTimerCreate(
		"AP timeout", //timer name
		SERVER_TIMEOUT/portTICK_PERIOD_MS, //timer period
		pdFALSE, //autoreload
		NULL, //timer ID
		server_timeoutFunction //callback function
		);
	if (xTimerStart(server_timeoutTimer, 0) != pdPASS)
		ESP_LOGE(TAG, "Failed to start timeout timer");
}

/*
	Resets the timeout timer
	Should be called everytime the configuration page sends something to the server
*/
void server_resetTimer() {
	if (xTimerReset(server_timeoutTimer, 0) != pdPASS) 
		ESP_LOGE(TAG, "Failed to reset timeout timer");
}

/*
	Deletes the timeout timer
	Should only be called when the server is stopped
*/
void server_deleteTimer() {
	if (xTimerDelete(server_timeoutTimer, 0) != pdPASS)
		ESP_LOGE(TAG, "Failed to delete timeout timer");
}

/*
	Initializes the SPI Flash FileSystem which holds the configuration page
*/
esp_err_t initSpiffs(void) {
	ESP_LOGI(TAG, "Initializing SPIFFS");

	esp_vfs_spiffs_conf_t conf = {
	  .base_path = SERVER_BASEPATH,
	  .partition_label = NULL,
	  .max_files = 5,   // This decides the maximum number of files that can be created on the storage
	  .format_if_mount_failed = true
	};

	esp_err_t ret = esp_vfs_spiffs_register(&conf);
	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		} else if (ret == ESP_ERR_NOT_FOUND) {
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		} else {
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
		}
		return ESP_FAIL;
	}

	size_t total = 0, used = 0;
	ret = esp_spiffs_info(NULL, &total, &used);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
		return ESP_FAIL;
	}

	ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
	return ESP_OK;
}

/*
	Server initialization function
	Initializes everything required for the server to function properly
*/
esp_err_t server_start() {
	//start spiffs
	ESP_ERROR_CHECK(initSpiffs());

	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	config.uri_match_fn = httpd_uri_match_wildcard;

	ESP_LOGI(TAG, "Starting server");
	if (httpd_start(&server, &config) != ESP_OK) {
		ESP_LOGE(TAG, "Server failed to start");
		return ESP_FAIL;
	}

	//register uri's
	httpd_uri_t ssidList = {
		.uri = "/getSSIDList",
		.method = HTTP_GET,
		.handler = server_getSSIDList,
		.user_ctx = NULL
	};
	httpd_register_uri_handler(server, &ssidList);

	httpd_uri_t currentSSID = {
		.uri = "/getCurrentSSID",
		.method = HTTP_GET,
		.handler = server_getCurrentSSID,
		.user_ctx = NULL
	};
	httpd_register_uri_handler(server, &currentSSID);

	httpd_uri_t connect = {
		.uri = "/connect",
		.method = HTTP_GET,
		.handler = server_connect,
		.user_ctx = NULL
	};
	httpd_register_uri_handler(server, &connect);

	httpd_uri_t getWifiState = {
		.uri = "/getWifiState",
		.method = HTTP_GET,
		.handler = server_getWifiState,
		.user_ctx = NULL
	};
	httpd_register_uri_handler(server, &getWifiState);

	httpd_uri_t sendFile = {
		.uri = "/*",
		.method = HTTP_GET,
		.handler = server_sendFile,
		.user_ctx = NULL
	};
	httpd_register_uri_handler(server, &sendFile);

	//start timeout timer
	server_startTimer();

	return ESP_OK;
}

/*
	Stops the webserver
*/
void server_stop() {
	if (server) {
		server_deleteTimer();
		esp_vfs_spiffs_unregister(NULL);
		httpd_stop(server);
		server = NULL;
	}
}

/* 
	Return the HTTP response content type according to file extension 
*/
char* server_setContentType(char* filename) {
	if (CHECK_FILE_EXT(filename, ".htm"))  {ESP_LOGI(TAG, "File type: .htm"); return "text/html";}
	if (CHECK_FILE_EXT(filename, ".html")) {ESP_LOGI(TAG, "File type: .html");return "text/html";}
	if (CHECK_FILE_EXT(filename, ".css"))  {ESP_LOGI(TAG, "File type: .css"); return "text/css";}
	if (CHECK_FILE_EXT(filename, ".js"))   {ESP_LOGI(TAG, "File type: .js");  return "application/javascript";}
	if (CHECK_FILE_EXT(filename, ".png"))  {ESP_LOGI(TAG, "File type: .png"); return "image/png";}
	if (CHECK_FILE_EXT(filename, ".gif"))  {ESP_LOGI(TAG, "File type: .gif"); return "image/gif";}
	if (CHECK_FILE_EXT(filename, ".jpg"))  {ESP_LOGI(TAG, "File type: .jpg"); return "image/jpeg";}
	if (CHECK_FILE_EXT(filename, ".ico"))  {ESP_LOGI(TAG, "File type: .ico"); return "image/x-icon";}
	if (CHECK_FILE_EXT(filename, ".xml"))  {ESP_LOGI(TAG, "File type: .xml"); return "text/xml";}
	if (CHECK_FILE_EXT(filename, ".pdf"))  {ESP_LOGI(TAG, "File type: .pdf"); return "application/x-pdf";}
	if (CHECK_FILE_EXT(filename, ".zip"))  {ESP_LOGI(TAG, "File type: .zip"); return "application/x-zip";}
	if (CHECK_FILE_EXT(filename, ".gz"))   {ESP_LOGI(TAG, "File type: .gz");  return "application/x-gzip";}

	ESP_LOGI(TAG, "File type: text");
	return "text/plain";
}

/*
	Get a list of available SSIDs
*/
esp_err_t server_getSSIDList(httpd_req_t* req) {
	server_resetTimer();
	int16_t res = wifi_scanNetworks(0, 0, 0, 300);
	if (res < 0) { //failed or running
		ESP_LOGE(TAG, "Failed to scan WiFi networks");
		httpd_resp_sendstr(req, "Scanning failed");
		return ESP_FAIL;
	}
	ESP_LOGI(TAG, "Networks scanned: found %d networks", res);
	int sent = 0;
	for (int i = 0; i < res; i++) {
		char* ssid = wifi_SSID(i);
		ESP_LOGI(TAG, "SSID %d: %s", i+1, (strcmp(ssid, "") == 0 ? "(empty)" : ssid));
		if (strcmp(ssid, "") != 0) { //filter out empty ssid names
			if (sent != 0)
				httpd_resp_sendstr_chunk(req, ",");
			httpd_resp_sendstr_chunk(req, ssid);
			sent++;
		}
	}
	httpd_resp_sendstr_chunk(req, NULL);
	return ESP_OK;
}

/*
	Get the currently configured SSID
*/
esp_err_t server_getCurrentSSID(httpd_req_t* req) {
	server_resetTimer();
	char* ssid = wifi_getStoredSSID();
	if (strlen(ssid) == 0)
		httpd_resp_sendstr(req, "-");
	else
		httpd_resp_sendstr(req, ssid);
	return ESP_OK;
}

/*
	Connect to the network given in the request parameters
*/
esp_err_t server_connect(httpd_req_t* req) {
	server_resetTimer();
	char* buf = server_getUrlQuery(req);
	if (buf != NULL) {
		ESP_LOGI(TAG, "URL query: %s", buf);
		char ssid[WIFI_MAX_SSID_LEN];
		char pass[WIFI_MAX_PASS_LEN];
		if (server_getArg(buf, "ssid", ssid, sizeof(ssid)) == ESP_OK) {
			ESP_LOGI(TAG, "ssid: %s", ssid);
		}
		if (server_getArg(buf, "pass", pass, sizeof(pass)) == ESP_OK) {
			ESP_LOGI(TAG, "pass: %s", pass);
		}
		if (ssid != NULL)  {//pass can be NULL for unprotected networks
			httpd_resp_sendstr_chunk(req, "Attempting to connect to ");
			httpd_resp_sendstr_chunk(req, ssid);
			httpd_resp_sendstr_chunk(req, NULL);

			wifi_setWifiSettings(ssid, pass);
			wifi_connectSTA();
		}
		else
			httpd_resp_sendstr(req, "No SSID received");
	}
	else {
		httpd_resp_sendstr(req, "No data received");
	}

	free(buf);
	return ESP_OK;
}

/*
	Get the current connection state
	Will be called repeatedly after server_connect() is called to track the connection state
*/
esp_err_t server_getWifiState(httpd_req_t* req) {
	server_resetTimer();
	switch (wifi_connectState()) {
		case WIFI_STATE_DISCONNECTED:
			httpd_resp_sendstr(req, "iDisconnected");
			break;
		case WIFI_STATE_CONNECTING:
			httpd_resp_sendstr(req, "iConnecting");
			break;
		case WIFI_STATE_CONNECTING_FAILED:
			httpd_resp_sendstr(req, "eFailed to connect");
			break;
		case WIFI_STATE_CONNECTED:
			httpd_resp_sendstr(req, "sConnected");
			break;
		default:
			httpd_resp_sendstr(req, "eUndefined");
			break;
	}
	return ESP_OK;
}

/*
	Send a file from the SPIFFS to the client or 404 if the file doesn't exist
*/
esp_err_t server_sendFile(httpd_req_t* req) {
	server_resetTimer();
	ESP_LOGI(TAG, "Requested: %s", req->uri);
	
	char filepath[SERVER_MAX_FILENAME];
	FILE* fd = NULL;
	struct stat file_stat;

	/* Retrieve the base path of file storage to construct the full path*/
	strcpy(filepath, SERVER_BASEPATH);

	/* Concatenate the requested file path*/
	strcat(filepath, req->uri);
	if (strcmp(req->uri, "/") == 0)
		strcat(filepath, "index.html");

	if (stat(filepath, &file_stat) == -1) {
		ESP_LOGE(TAG, "Failed to stat file : %s", filepath);
		/* Respond with 404 Not Found*/
		httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File does not exist");
		return ESP_FAIL;
	}

	fd = fopen(filepath, "r");
	if (!fd) {
		ESP_LOGE(TAG, "Failed to read existing file : %s", filepath);
		/* Respond with 500 Internal Server Error*/
		httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
		return ESP_FAIL;
	}

	ESP_LOGI(TAG, "Sending file : %s (%ld bytes)...", filepath, file_stat.st_size);
	httpd_resp_set_type(req, server_setContentType(filepath));

	/* Retrieve the pointer to scratch buffer for temporary storage*/
	size_t tempSize = 4096;
	char* chunk = (char*)malloc(tempSize*sizeof(char));
	size_t chunkSize = 0;
	if (chunk == NULL) {
		ESP_LOGE(TAG, "Malloc failed. Can't send file");
		httpd_resp_sendstr_chunk(req, NULL);
		return ESP_ERR_NO_MEM;
	}
	do {
		/* Read file in chunks into the scratch buffer*/
		chunkSize = fread(chunk, 1, tempSize, fd);
		ESP_LOGI(TAG, "Sending chunk containing %u bytes", chunkSize);

		/* Send the buffer contents as HTTP response chunk*/
		if (httpd_resp_send_chunk(req, chunk, chunkSize) != ESP_OK) {
			fclose(fd);
			ESP_LOGE(TAG, "File sending failed!");
			/* Abort sending file*/
			httpd_resp_sendstr_chunk(req, NULL);
			/* Respond with 500 Internal Server Error*/
			httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
			return ESP_FAIL;
		}

		/* Keep looping till the whole file is sent*/
	} while (chunkSize != 0);

	/* Close file after sending complete*/
	fclose(fd);
	ESP_LOGI(TAG, "File sending complete");

	/* Respond with an empty chunk to signal HTTP response completion*/
	httpd_resp_sendstr_chunk(req, NULL);
	return ESP_OK;
}

/*
	Allocates and fills a char buffer containing the url query data
	The buffer needs to be freed using free() when done
	Returns a pointer when the data is successfully retrieved, otherwise NULL
*/
char* server_getUrlQuery(httpd_req_t* req) {
	uint16_t bufSize = httpd_req_get_url_query_len(req) + 1; //length + 1 for '\0'
	char* buf = (char*)malloc(bufSize*sizeof(char));
	if (httpd_req_get_url_query_str(req, buf, bufSize) == ESP_OK)
		return buf;
	return NULL;
}

/*
	Retrieves the given key from the given buffer and decodes it
	Returns ESP_OK on success, otherwise ESP_FAIL
*/
esp_err_t server_getArg(char* buf, char* key, char* value, size_t len) {
	if (!buf)
		return ESP_FAIL;
	
	if (httpd_query_key_value(buf, key, value, len) == ESP_OK) {
		server_decodeURL(value, len);
		return ESP_OK;
	}
	return ESP_FAIL;
}

/*
	Decodes a HTML encoded string
*/
void server_decodeURL(char* data, uint16_t strLen) {
	uint16_t decPos = 0;
	char tmp[] = "0x00";
	for (uint16_t i = 0; i < strLen; i++) {
		switch (data[i]) {
			case '%':
				if ((i + 1) < strLen) {
					tmp[2] = data[++i];
					tmp[3] = data[++i];
					data[decPos] = strtol(tmp, NULL, 16);
				}
				break;
			case '+':
				data[decPos] = ' ';
				break;
			default:
				data[decPos] = data[i];
				break;
		}
		decPos++;
	}
	if (decPos < strLen)
		data[decPos] = '\0';
}

