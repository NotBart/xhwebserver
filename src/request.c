/*
Filename: request.c
Author: Bartoloměj Bičovský
Purpose: handle request
Version: 0.0.1
*/

#include "request.h"
#include "options.h"
#include "urlencode.h"
#include "files.h"
#include "response.h"
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

struct requestBodyParseResult parseRequest(int fd) {
    struct requestBodyParseResult returnValues;

    char request[MAX_REQUEST_SIZE];
    if (read(fd, request, MAX_REQUEST_SIZE) < 0) {
        perror("read");
        returnValues.errorlevel = 500;
        return returnValues;
    }

    returnValues.pointnine = true;
    int reqlen = strlen(request);
    for (int i = 0; i <= reqlen - 7; i++) {
        if (!strncmp(&request[i], " HTTP/1", 7)) {
            returnValues.pointnine = false;
            break;
        }
    }

    char httpMethodStr[32];
    for (int i = 0; i < 32; i++) {
        if (request[i] == ' ') {
            httpMethodStr[i] = '\0';
            break;
        } else {
            httpMethodStr[i] = request[i];
        }

        if (i == 31 && request[i] != ' ') {
            returnValues.errorlevel = 414;
            return returnValues;
        }
    }

    if (!strcmp(httpMethodStr, "GET")) {
        returnValues.method = GET;
    } else if (!strcmp(httpMethodStr, "POST")) {
        returnValues.method = POST;
    } else if (!strcmp(httpMethodStr, "HEAD")) {
        returnValues.method = HEAD;
    } else if (!strcmp(httpMethodStr, "PUT")) {
        returnValues.method = PUT;
    } else if (!strcmp(httpMethodStr, "DELETE")) {
        returnValues.method = DELETE;
    } else if (!strcmp(httpMethodStr, "CONNECT")) {
        returnValues.method = CONNECT;
    } else if (!strcmp(httpMethodStr, "OPTIONS")) {
        returnValues.method = OPTIONS;
    } else if (!strcmp(httpMethodStr, "TRACE")) {
        returnValues.method = TRACE;
    } else if (!strcmp(httpMethodStr, "PATCH")) {
        returnValues.method = PATCH;
    } else {
        returnValues.errorlevel = 501;
        return returnValues;
    } 
    
    char httpUrlStr[2048];
    bool requestHasGetParams = false;
    int requestGetParamsStart = -1;
    int lastParsed = strlen(httpMethodStr);
    int j = 0;
    for (int i = lastParsed + 1; i < lastParsed + 2049; i++) {
        if (request[i] == ' ' || request[i] == '?' || request[i] == '#') {
            httpUrlStr[j] = '\0';
            if (request[i] == '?' && !(request[i+1] == '\0')) {
                requestHasGetParams = true;
                requestGetParamsStart = i;
            }
            break;
        } else {
            httpUrlStr[j] = request[i];
            j++;
        }

        if (j == 2048 && httpUrlStr[j] != ' ') {
            returnValues.errorlevel = 414;
            return returnValues;
        }
    }
    strcpy(returnValues.url, httpUrlStr);

    returnValues.querycount = 0;
    if (returnValues.method == GET && requestHasGetParams) {
        bool currentStringIsKey, isQuestionLiteral;
        currentStringIsKey = isQuestionLiteral = false;
        char currentString[MAX_VALUE_SIZE];
        int currentStringLength = 0;
        for (int i = requestGetParamsStart; i < MAX_REQUEST_SIZE; i++) {
            switch (request[i]) {
                case '?':
                    if (!isQuestionLiteral) {
                        isQuestionLiteral = true;
                        currentStringIsKey = true;
                        currentStringLength = 0;
                        memset(currentString, '\0', MAX_VALUE_SIZE);
                        returnValues.querycount++;
                    } else {
                        currentString[currentStringLength++] = '?';
                    }
                    break;

                case '&':
                    if (currentStringLength) {
                        if (currentStringIsKey) {
                            strncpy(returnValues.queries[returnValues.querycount-1].k, currentString, MAX_KEY_SIZE-1);
                            returnValues.queries[returnValues.querycount].k[MAX_KEY_SIZE-1] = '\0';
                            strncpy(returnValues.queries[returnValues.querycount-1].v, "EmptyVal\0", 9);
                        } else {
                            strncpy(returnValues.queries[returnValues.querycount-1].v, currentString, MAX_VALUE_SIZE-1);
                            returnValues.queries[returnValues.querycount].v[MAX_VALUE_SIZE-1] = '\0';
                        }
                        returnValues.querycount++;
                    }
                    currentString[currentStringLength] = '\0';
                    currentStringIsKey = true;
                    currentStringLength = 0;
                    memset(currentString, '\0', MAX_VALUE_SIZE);
                    break;

                case '=':
                    if (currentStringIsKey) {
                        if (currentStringLength) {
                            strncpy(returnValues.queries[returnValues.querycount-1].k, currentString, MAX_KEY_SIZE-1);
                            returnValues.queries[returnValues.querycount].k[MAX_KEY_SIZE-1] = '\0';
                        } else {
                            strncpy(returnValues.queries[returnValues.querycount-1].k, "EmptyKey\0", 9);
                        }
                        currentString[currentStringLength] = '\0';
                        currentStringIsKey = false;
                        currentStringLength = 0;
                        memset(currentString, '\0', MAX_VALUE_SIZE);
                    } else {
                        currentString[currentStringLength++] = '=';
                    }
                    break;

                case '\r':
                    if (i + 1 < MAX_REQUEST_SIZE && request[i+1] != '\n') {
                        break;
                    }
                case '\0':
                case '\n':
                case ' ':
                    if (currentStringLength) {
                        if (currentStringIsKey) {
                            strncpy(returnValues.queries[returnValues.querycount-1].k, currentString, MAX_KEY_SIZE-1);
                            returnValues.queries[returnValues.querycount].k[MAX_KEY_SIZE-1] = '\0';
                            strncpy(returnValues.queries[returnValues.querycount-1].v, "EmptyVal\0", 9);
                        } else {
                            strncpy(returnValues.queries[returnValues.querycount-1].v, currentString, MAX_VALUE_SIZE-1);
                            returnValues.queries[returnValues.querycount].v[MAX_VALUE_SIZE-1] = '\0';
                        }
                    }

                    lastParsed = i;
                    if (request[lastParsed] == '\r') { lastParsed++; }
                    goto exitGetParseLoop;
                    break;

                default:
                    currentString[currentStringLength++] = request[i];
                    break;
            
            }

            if (returnValues.querycount > 128) {
                returnValues.errorlevel = 414;
                return returnValues;
            }

            if (currentStringIsKey) {
                if (currentStringLength >= MAX_KEY_SIZE - 1) {
                    returnValues.errorlevel = 414;
                    return returnValues;
                }
            } else {
                if (currentStringLength >= MAX_VALUE_SIZE - 1) {
                    returnValues.errorlevel = 414;
                    return returnValues;
                }
            }
        }
        exitGetParseLoop:;
    }

    while (request[lastParsed] != '\n' && request[lastParsed] != '\0') { lastParsed++; }
    
    returnValues.headercount = 0;
    if (request[lastParsed] == '\n') {
        bool isHeaderKey = true;
        char currentString[MAX_VALUE_SIZE];
        memset(currentString, '\0', MAX_VALUE_SIZE);
        int currentStringLength = 0;
        for (int i = ++lastParsed; i < MAX_REQUEST_SIZE; i++) {
            switch (request[i]) {
                case ':':
                    if (isHeaderKey) {
                        if (i + 1 < MAX_REQUEST_SIZE && request[i+1] == ' ') {
                            strncpy(returnValues.headers[returnValues.headercount].k, currentString, MAX_KEY_SIZE-1);
                            returnValues.headers[returnValues.headercount].k[MAX_KEY_SIZE-1] = '\0';
                            memset(currentString, '\0', MAX_VALUE_SIZE);
                            isHeaderKey = false;
                            currentStringLength = 0;
                            if (i + 1 < MAX_REQUEST_SIZE && request[i+1] == ' ') { i++; }
                        } else {
                            returnValues.errorlevel = 400;
                            return returnValues;
                        }
                    } else {
                        if (currentStringLength < MAX_VALUE_SIZE - 1) {
                            currentString[currentStringLength++] = request[i];
                        }
                    }
                    break;

                case '\r':
                    if (i + 1 < MAX_REQUEST_SIZE && request[i+1] != '\n') {
                        break;
                    }
                    i++;
                case '\0':
                case '\n':
                    if (currentStringLength == 0 && isHeaderKey) { goto exitHeaderParseLoop; }
                    if (!isHeaderKey) {
                        strncpy(returnValues.headers[returnValues.headercount].v, currentString, MAX_VALUE_SIZE-1);
                        returnValues.headers[returnValues.headercount].v[MAX_VALUE_SIZE-1] = '\0';
                        memset(currentString, '\0', MAX_VALUE_SIZE);
                        returnValues.headercount++;
                        isHeaderKey = true;
                        currentStringLength = 0;
                    } else {
                        returnValues.errorlevel = 400;
                        return returnValues;
                    }
                    if (request[i] == '\0') { goto exitHeaderParseLoop; }
                    break;

                default:
                    if (isHeaderKey) {
                        if (currentStringLength < MAX_KEY_SIZE - 1) {
                            currentString[currentStringLength++] = request[i];
                        } else {
                            returnValues.errorlevel = 413;
                            return returnValues;
                        }
                    } else {
                        if (currentStringLength < MAX_VALUE_SIZE - 1) {
                            currentString[currentStringLength++] = request[i];
                        } else {
                            returnValues.errorlevel = 413;
                            return returnValues;
                        }
                    }
                    break;
            }
        }
        exitHeaderParseLoop:;
    }

    return returnValues;
}

char *getIP(int fd) {
    struct sockaddr_storage addr;
    int addrsize = sizeof(struct sockaddr_storage);

    if (getpeername(fd, (struct sockaddr *) &addr, &addrsize)) {
        perror("[request.c] getpeername");
        return "whoops epick failurez !! ! fgsdhnjikodhfgbyjniokdfgyh";
    }

    void *ipaddr;
    struct sockaddr_in6 *ins = (struct sockaddr_in6 *) &addr;
    static char ipstr[INET6_ADDRSTRLEN];
    inet_ntop(addr.ss_family, &(ins->sin6_addr), ipstr, sizeof(ipstr));
    return ipstr;
}

int wslog(char *path, struct requestBodyParseResult request, int fd, int status, char *ipstr) {
    printf("%d %s ", status, ipstr);

    switch (request.method) {
        case GET:
            printf("GET ");
            break;

        case POST:
            printf("POST ");
            break;

        case HEAD:
            printf("HEAD ");
            break;
            
        case PUT:
            printf("PUT ");
            break;
            
        case DELETE:
            printf("DELETE ");
            break;
            
        case CONNECT:
            printf("CONNECT ");
            break;
            
        case OPTIONS:
            printf("OPTIONS ");
            break;
            
        case TRACE:
            printf("TRACE ");
            break;
            
        case PATCH:
            printf("PATCH ");
            break;
            
        default:
            printf("ERR ");
            break;
    }

    printf("%s\n", path);
}

int handleRequest(int fd) {
    struct requestBodyParseResult parsedRequest = parseRequest(fd);
    char requestedPath[MAX_URL_PATH*3+1];
    int replystatus = 0;
    char *remoteIP = getIP(fd);
    strncpy(requestedPath, urlDecode(parsedRequest.url, true), MAX_URL_PATH * 3 + 1);
    if(!parsedRequest.errorlevel && strcmp(requestedPath, "INVALID!!!!")) {
        if (parsedRequest.method == GET) {
            if (isPathValid(requestedPath, true)) {
                char *mimetype = getMimetype(requestedPath);
                if (!strcmp(mimetype, "cgiscript")) {
                    evaluateCgi(requestedPath, fd, parsedRequest, remoteIP);
                    replystatus = 777;
                } else {
                    if (!parsedRequest.pointnine) { 
                        writeSuccessHeader(fd, requestedPath, mimetype);
                    }
                    serveFile(requestedPath, fd);
                    replystatus = 200;
                }
            } else {
                if (parsedRequest.pointnine) {
                    writePointNineError(fd, 404);
                    replystatus = 404;
                } else {
                    writeError(fd, 404, true);
                    replystatus = 404;
                }
            }
        } else if (parsedRequest.method == HEAD) {
            if (isPathValid(requestedPath, true)) {
                char *mimetype = getMimetype(requestedPath);
                if (!strcmp(mimetype, "cgiscript")) {
                    evaluateCgi(requestedPath, fd, parsedRequest, remoteIP);
                    replystatus = 777;
                } else {
                    writeSuccessHeader(fd, requestedPath, mimetype);
                    replystatus = 200;
                }
            } else {
                writeError(fd, 404, false);
                replystatus = 404;
            }
        } else if (parsedRequest.method > -1 && parsedRequest.method < 9) {
            if (isPathValid(requestedPath, true)) {
                char *mimetype = getMimetype(requestedPath);
                if (!strcmp(mimetype, "cgiscript")) {
                    evaluateCgi(requestedPath, fd, parsedRequest, remoteIP);
                    replystatus = 777;
                } else {
                    writeError(fd, 405, true);
                    replystatus = 405;
                }
            } else {
                writeError(fd, 404, true);
                replystatus = 405;
            }
        }
    } else {
        if (parsedRequest.method == GET && parsedRequest.pointnine) {
            writePointNineError(fd, parsedRequest.errorlevel);
        } else {
            if (parsedRequest.method == HEAD) {
                writeError(fd, parsedRequest.errorlevel, false);
            } else {
                writeError(fd, parsedRequest.errorlevel, true);
            }
        }
        replystatus = parsedRequest.errorlevel;
    }

    wslog(requestedPath, parsedRequest, fd, replystatus, remoteIP);

    shutdown(fd, SHUT_WR);
    close(fd);
    return 0;
}