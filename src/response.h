/*
Filename: response.h
Author: Bartoloměj Bičovský
Purpose: response header
Version: 0.0.1
*/

#ifndef RESPONSE_H
#define RESPONSE_H

#include <stdbool.h>

struct httpResponse {
    int code;
    const char *text;
};

int writeSuccessHeader(int fd, char *path, char *mimetype);
int writeError(int fd, int error, bool writebody);
int writePointNineError(int fd, int error);

#endif