/*
Filename: files.h
Author: Bartoloměj Bičovský
Purpose: files header
Version: 0.0.1
*/

#ifndef FILES_H
#define FILES_H

#include "request.h"
#include <stdbool.h>

bool isPathValid(char *path, bool inwebroot);
char *getMimetype(char *path);
int serveFile(char *path, int fd);
int evaluateCgi(char *path, int fd, struct requestBodyParseResult request, char *remoteip);

#endif