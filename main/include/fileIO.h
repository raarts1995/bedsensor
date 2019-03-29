#ifndef FILEIO_H
#define FILEIO_H

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/unistd.h>

#include "esp_vfs.h"

#include "esp_err.h"
#include "esp_log.h"

char* fileIO_constructPath(char* base, char* path);
bool fileIO_fileExists(char* base, char* path);
size_t fileIO_getFileSize(char* base, char* path);
FILE* fileIO_openFile(char* base, char* path, char* mode);
size_t fileIO_readFile(FILE* f, char* chunk, size_t chunkSize);
void fileIO_writeFile(FILE* f, char* chunk);
void fileIO_closeFile(FILE* f);

#endif