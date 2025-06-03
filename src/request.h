/*
Filename: request.h
Author: Bartoloměj Bičovský
Purpose: request header
Version: 0.0.1
*/

#ifndef REQUEST_H
#define REQUEST_H

#include "options.h"
#include <stdbool.h>

enum httpMethod {
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE,
    PATCH,
    ERROR = -1
};

struct httpParam {
    char k[MAX_KEY_SIZE];
    char v[MAX_VALUE_SIZE];
};

struct requestBodyParseResult {
    int errorlevel;
    bool pointnine;
    enum httpMethod method;
    char url[MAX_URL_PATH];
    int querycount;
    struct httpParam queries[128];
    int headercount;
    struct httpParam headers[128];
};

int handleRequest(int fd);

#endif