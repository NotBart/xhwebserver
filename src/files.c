/*
Filename: files.c
Author: Bartoloměj Bičovský
Purpose: handles files, cgi
Version: 0.0.1a
*/

#include "files.h"
#include "options.h"
#include "request.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>

bool isPathValid(char *path, bool inwebroot) {
    if (strstr(path, "..")) {
        return false;
    }

    char finalPath[4096];
    if (inwebroot) {
        strcpy(finalPath, WEB_ROOT);
        strcat(finalPath, path);
    } else {
        strcpy(finalPath, path);
    }

    struct stat fileStats;
    if (stat(finalPath, &fileStats) != 0) {
        return false;
    }

    if (inwebroot && S_ISDIR(fileStats.st_mode)) {
        char newpath[4096];
        strcpy(newpath, path);
        strcat(newpath, "/index.html");

        if (isPathValid(newpath, true)) {
            strcat(path, "/index.html");
            return true;
        }
    }
    
    if (access(finalPath, R_OK) || !S_ISREG(fileStats.st_mode)) {
        return false;
    }

    return true;
}

char *getMimetype(char *path) {
    char *lastdot = strrchr(path, '.');

    if (!strcmp(lastdot, ".html") || !strcmp(lastdot, ".htm")) {
        return "text/html;charset=utf-8";
    } else if (!strcmp(lastdot, ".cgi")) {
        return "cgiscript";
    } else if (!strcmp(lastdot, ".css")) {
        return "text/css;charset=utf-8";
    } else if (!strcmp(lastdot, ".js")) {
        return "application/javascript;charset=utf-8";
    } else if (!strcmp(lastdot, ".json")) {
        return "application/json;charset=utf-8";
    } else if (!strcmp(lastdot, ".xml")) {
        return "application/xml;charset=utf-8";
    } else if (!strcmp(lastdot, ".jpg") || !strcmp(lastdot, ".jpeg")) {
        return "image/jpeg";
    } else if (!strcmp(lastdot, ".png")) {
        return "image/png";
    } else if (!strcmp(lastdot, ".gif")) {
        return "image/gif";
    } else if (!strcmp(lastdot, ".svg")) {
        return "image/svg+xml;charset=utf-8";
    } else if (!strcmp(lastdot, ".webp")) {
        return "image/webp";
    } else if (!strcmp(lastdot, ".ico")) {
        return "image/x-icon";
    } else if (!strcmp(lastdot, ".mp3")) {
        return "audio/mpeg";
    } else if (!strcmp(lastdot, ".ogg")) {
        return "audio/ogg";
    } else if (!strcmp(lastdot, ".wav")) {
        return "audio/wav";
    } else if (!strcmp(lastdot, ".mid") || !strcmp(lastdot, ".midi")) {
        return "audio/midi";
    } else if (!strcmp(lastdot, ".mp4")) {
        return "video/mp4";
    } else if (!strcmp(lastdot, ".webm")) {
        return "video/webm";
    } else if (!strcmp(lastdot, ".ogg")) {
        return "application/ogg";
    } else if (!strcmp(lastdot, ".pdf")) {
        return "application/pdf";
    } else if (!strcmp(lastdot, ".zip")) {
        return "application/zip";
    } else if (!strcmp(lastdot, ".woff")) {
        return "font/woff";
    } else if (!strcmp(lastdot, ".woff2")) {
        return "font/woff2";
    } else if (!strcmp(lastdot, ".ttf")) {
        return "font/ttf";
    } else if (!strcmp(lastdot, ".otf")) {
        return "font/otf";
    } else if (!strcmp(lastdot, ".txt")) {
        return "text/plain;charset=utf-8";
    } else if (!strcmp(lastdot, ".csv")) {
        return "text/csv;charset=utf-8";
    } else {
        return "application/octet-stream";
    }
}

int serveFile(char *path, int fd) {
    char fileReadBuf[16384];
    
    char finalPath[4096];
    strcpy(finalPath, WEB_ROOT);
    strcat(finalPath, path);

    int readbytes;
    int filefd = open(finalPath, O_RDONLY);

    while ((readbytes = read(filefd, fileReadBuf, 16384)) > 0) {
        write(fd, fileReadBuf, readbytes);
    }

    return 1;
}

int evaluateCgi(char *path, int fd, struct requestBodyParseResult request, char *remoteip) {
    char finalPath[4096];
    strcpy(finalPath, WEB_ROOT);
    strcat(finalPath, path);

    int pipefd[2];
    pipe(pipefd);

    int subprocess = fork();

    if (subprocess == 0) {
        close(pipefd[0]);

        setenv("SERVER_SOFTWARE", SERVER_BANNER, 1);
        setenv("GATEWAY_INTERFACE", "CGI/0", 1);
        if (request.pointnine) { 
            setenv("SERVER_PROTOCOL", "HTTP/0.9", 1);
        } else {
            setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
        }
        char portstr[6];
        sprintf(portstr, "%d", WEB_PORT);
        setenv("SERVER_PORT", portstr, 1);
        setenv("SCRIPT_NAME", path, 1);
        setenv("REMOTE_ADDR", remoteip, 1);
        switch (request.method) {
            case GET:
                setenv("HTTP_METHOD", "GET", 1);
                break;

            case POST:
                setenv("HTTP_METHOD", "POST", 1);
                break;

            case HEAD:
                setenv("HTTP_METHOD", "HEAD", 1);
                break;
                
            case PUT:
                setenv("HTTP_METHOD", "PUT", 1);
                break;
                
            case DELETE:
                setenv("HTTP_METHOD", "DELETE", 1);
                break;
                
            case CONNECT:
                setenv("HTTP_METHOD", "CONNECT", 1);
                break;
                
            case OPTIONS:
                setenv("HTTP_METHOD", "OPTIONS", 1);
                break;
                
            case TRACE:
                setenv("HTTP_METHOD", "TRACE", 1);
                break;
                
            case PATCH:
                setenv("HTTP_METHOD", "PATCH", 1);
                break;
                
            default:
                setenv("HTTP_METHOD", "ERR", 1);
                break;
        }

        int x = 7;
        char currstr[MAX_KEY_SIZE+8];
        for (int i = 0; i < request.headercount; i++) {
            x = 7;
            memset(currstr, '\0', MAX_KEY_SIZE+8);
            strcpy(currstr, "HEADER_");
            for (int j = 0; j < MAX_KEY_SIZE; j++) {
                char currchar = request.headers[i].k[j];
                if (currchar == '-') {
                    currstr[x++] = '_';
                } else if (currchar >= 'a' && currchar <= 'z') {
                    currstr[x++] = currchar - 32; 
                } else if ((currchar >= 'A' && currchar <= 'Z') || (currchar >= '0' && currchar <= '9') || currchar == '_') {
                    currstr[x++] = currchar;
                } else if (currchar == '\0') {
                    currstr[x] = '\0';
                }
            }
            setenv(currstr, request.headers[i].v, 1);
        }

        for (int i = 0; i < request.querycount; i++) {
            x = 6;
            memset(currstr, '\0', MAX_KEY_SIZE+8);
            strcpy(currstr, "QUERY_");
            for (int j = 0; j < MAX_KEY_SIZE; j++) {
                char currchar = request.queries[i].k[j];
                if (currchar == '-') {
                    currstr[x++] = '_';
                } else if (currchar >= 'a' && currchar <= 'z') {
                    currstr[x++] = currchar - 32; 
                } else if ((currchar >= 'A' && currchar <= 'Z') || (currchar >= '0' && currchar <= '9') || currchar == '_') {
                    currstr[x++] = currchar;
                } else if (currchar == '\0') {
                    currstr[x] = '\0';
                }
            }
            setenv(currstr, request.queries[i].v, 1);
        }

        if (dup2(pipefd[1], STDOUT_FILENO) < 0) {
            exit(EXIT_FAILURE);
        }

        close(pipefd[1]);

        char *args[] = {finalPath, NULL};
        execv(finalPath, args);
        exit(EXIT_SUCCESS);
    } else {
        close(pipefd[1]);

        int substatus;
        int readbytes;
        char buffer[4096];

        while ((readbytes = read(pipefd[0], buffer, 4096)) > 0) {
            if (write(fd, buffer, readbytes) < 0) {
                kill(subprocess, SIGKILL);
                break;
            }
        }

        close(pipefd[0]);
        waitpid(subprocess, &substatus, 0);
    }

    return 0;
}
