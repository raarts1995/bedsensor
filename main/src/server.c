#include "server.h"

httpd_handle_t server = NULL;

/* Function to initialize SPIFFS */
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

	httpd_uri_t wifiState = {
		.uri = "wifiState",
		.method = HTTP_GET,
		.handler = server_wifiState,
		.user_ctx = NULL
	};
	httpd_register_uri_handler(server, &wifiState);

	httpd_uri_t sendFile = {
		.uri = "/*",
		.method = HTTP_GET,
		.handler = server_sendFile,
		.user_ctx = NULL
	};
	httpd_register_uri_handler(server, &sendFile);

	return ESP_OK;
}

void server_stop() {
	if (server) {
		httpd_stop(server);
		server = NULL;
	}
}

/* Set HTTP response content type according to file extension */
char *server_setContentType(char *filename) {
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

esp_err_t server_getSSIDList(httpd_req_t *req) {
	int16_t res = wifi_scanNetworks(0, 0, 0, 300);
	if (res < 0) { //failed or running
		ESP_LOGE(TAG, "Failed to scan WiFi networks");
		httpd_resp_sendstr(req, "Scanning failed");
		return ESP_FAIL;
	}
	ESP_LOGI(TAG, "Networks scanned: found %d networks", res);
	int sent = 0;
	for (int i = 0; i < res; i++) {
		char *ssid = wifi_SSID(i);
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

esp_err_t server_getCurrentSSID(httpd_req_t *req) {
	httpd_resp_sendstr(req, BS_WIFI_SSID);
	return ESP_OK;
}

esp_err_t server_connect(httpd_req_t *req) {
	char *buf = server_getUrlQuery(req);
	if (buf != NULL) {
		ESP_LOGI(TAG, "URL query: %s", buf);
		char ssid[32];
		char pass[32];
		if (server_getArg(buf, "ssid", ssid, sizeof(ssid)) == ESP_OK) {
			ESP_LOGI(TAG, "ssid: %s", ssid);
		}
		if (server_getArg(buf, "pass", pass, sizeof(pass)) == ESP_OK) {
			ESP_LOGI(TAG, "pass: %s", pass);
		}
		if (ssid != NULL)  {//pass can be NULL for unprotected networks
			wifi_connectSTA(ssid, pass);
			httpd_resp_sendstr_chunk(req, "Attempting to connect to ");
			httpd_resp_sendstr_chunk(req, ssid);
			httpd_resp_sendstr_chunk(req, NULL);
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

esp_err_t server_wifiState(httpd_req_t *req) {
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

esp_err_t server_sendFile(httpd_req_t *req) {
	ESP_LOGI(TAG, "Requested: %s", req->uri);
	
	char filepath[SERVER_MAX_FILENAME];
	FILE *fd = NULL;
	struct stat file_stat;

	/* Retrieve the base path of file storage to construct the full path */
	strcpy(filepath, SERVER_BASEPATH);

	/* Concatenate the requested file path */
	strcat(filepath, req->uri);
	if (strcmp(req->uri, "/") == 0)
		strcat(filepath, "index.html");

	if (stat(filepath, &file_stat) == -1) {
		ESP_LOGE(TAG, "Failed to stat file : %s", filepath);
		/* Respond with 404 Not Found */
		httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File does not exist");
		return ESP_FAIL;
	}

	fd = fopen(filepath, "r");
	if (!fd) {
		ESP_LOGE(TAG, "Failed to read existing file : %s", filepath);
		/* Respond with 500 Internal Server Error */
		httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
		return ESP_FAIL;
	}

	ESP_LOGI(TAG, "Sending file : %s (%ld bytes)...", filepath, file_stat.st_size);
	httpd_resp_set_type(req, server_setContentType(filepath));

	/* Retrieve the pointer to scratch buffer for temporary storage */
	size_t tempSize = 4096;
	char *chunk = (char *)malloc(tempSize*sizeof(char));
	size_t chunkSize = 0;
	if (chunk == NULL) {
		ESP_LOGE(TAG, "Malloc failed. Can't send file");
		httpd_resp_sendstr_chunk(req, NULL);
		return ESP_ERR_NO_MEM;
	}
	do {
		/* Read file in chunks into the scratch buffer */
		chunkSize = fread(chunk, 1, tempSize, fd);
		ESP_LOGI(TAG, "Sending chunk containing %u bytes", chunkSize);

		/* Send the buffer contents as HTTP response chunk */
		if (httpd_resp_send_chunk(req, chunk, chunkSize) != ESP_OK) {
			fclose(fd);
			ESP_LOGE(TAG, "File sending failed!");
			/* Abort sending file */
			httpd_resp_sendstr_chunk(req, NULL);
			/* Respond with 500 Internal Server Error */
			httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
			return ESP_FAIL;
		}

		/* Keep looping till the whole file is sent */
	} while (chunkSize != 0);

	/* Close file after sending complete */
	fclose(fd);
	ESP_LOGI(TAG, "File sending complete");

	/* Respond with an empty chunk to signal HTTP response completion */
	httpd_resp_sendstr_chunk(req, NULL);
	return ESP_OK;
}

/*
	Allocates and fills a char buffer containing the url query data
	The buffer needs to be freed using free() when done
	Returns a pointer when the data is successfully retrieved, otherwise NULL
*/
char *server_getUrlQuery(httpd_req_t *req) {
	uint16_t bufSize = httpd_req_get_url_query_len(req) + 1; //length + 1 for '\0'
	char *buf = (char *)malloc(bufSize*sizeof(char));
	if (httpd_req_get_url_query_str(req, buf, bufSize) == ESP_OK)
		return buf;
	return NULL;
}

/*
	Retrieves the given key from the given buffer and decodes it
	Returns ESP_OK on success, otherwise ESP_FAIL
*/
esp_err_t server_getArg(char *buf, char *key, char *value, size_t len) {
	if (!buf)
		return ESP_FAIL;
	
	if (httpd_query_key_value(buf, key, value, len) == ESP_OK) {
		server_decodeURL(value, len);
		return ESP_OK;
	}
	return ESP_FAIL;
}

void server_decodeURL(char *data, uint16_t strLen) {
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

