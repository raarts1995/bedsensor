#include "fileIO.h"
#define TAG "fileIO"

char* fileIO_constructPath(char* base, char* path) {
	size_t len = strlen(base) + strlen(path) + 1;
	char* filepath = (char*)malloc(len);
	if (filepath == NULL) {
		ESP_LOGE(TAG, "Failed to allocate memory");
	}
	memset(filepath, '\0', len);

	if (base[0] != '/')
		strcat(filepath, "/");
	strcat(filepath, base);

	if (path[0] != '/')
		strcat(filepath, "/");
	strcat(filepath, path);

	ESP_LOGI(TAG, "File path: %s", filepath);

	return filepath;
}

/*
	Checks if the given file exists
*/
bool fileIO_fileExists(char* base, char* path) {
	ESP_LOGI(TAG, "Check if file exists");
	struct stat file_stat;
	char* filepath = fileIO_constructPath(base, path);
	if (stat(filepath, &file_stat) == -1) {
		ESP_LOGE(TAG, "Failed to stat file : %s", filepath);
		free(filepath);
		return false;
	}
	free(filepath);
	return true;
}

/*
	Returns the size of a file
*/
size_t fileIO_getFileSize(char* base, char* path) {
	ESP_LOGI(TAG, "Getting file size");
	struct stat file_stat;
	char* filepath = fileIO_constructPath(base, path);
	if (stat(filepath, &file_stat) == -1) {
		ESP_LOGE(TAG, "Failed to stat file : %s", filepath);
		free(filepath);
		return 0;
	}
	free(filepath);
	return file_stat.st_size;
}

/*
	Opens the requested file
	Returns the file pointer
*/
FILE* fileIO_openFile(char* base, char* path, char* mode) {
	ESP_LOGI(TAG, "Opening file");
	char* filepath = fileIO_constructPath(base, path);

	FILE* f = fopen(filepath, mode);
	if (!f) {
		ESP_LOGE(TAG, "Failed to open file: %s", filepath);
		free(filepath);
		return NULL;
	}
	free(filepath);
	return f;
}

/*
	Reads the file and stores the content in the variable chunk
	Returns the amount of bytes read
*/
size_t fileIO_readFile(FILE* f, char* chunk, size_t chunkSize) {
	if (f)
		return fread(chunk, 1, chunkSize, f);
	return 0;
}

void fileIO_writeFile(FILE* f, char* chunk) {
	if (f)
		fprintf(f, chunk);
}

/*
	Closes the opened file
*/
void fileIO_closeFile(FILE* f) {
	if (f)
		fclose(f);
}