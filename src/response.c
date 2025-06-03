/*
Filename: response.c
Author: Bartoloměj Bičovský
Purpose: handle response
Version: 0.0.1
*/

#include "response.h"
#include "options.h"
#include "files.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>

struct httpResponse responses[] = {
    {404, "Not Found"},
    {414, "URI Too Long"},
    {413, "Payload Too Large"},
    {500, "Internal Server Error"},
    {501, "Not Implemented"},
    {400, "Bad Request"},
    {405, "Method Not Allowed"}
};

int writeSuccessHeader(int fd, char *path, char *mimetype) {
    char finalPath[4096];
    strcpy(finalPath, WEB_ROOT);
    strcat(finalPath, path);

    struct stat fileStats;
    if (stat(finalPath, &fileStats) == 0) {
        char headertemplate[] = "HTTP/1.1 200 OK\r\n"
                                "Server: " SERVER_BANNER "\r\n"
                                "Content-type: ";
        write(fd, headertemplate, strlen(headertemplate));
        write(fd, mimetype, strlen(mimetype));
        write(fd, "\r\nContent-Length: ", strlen("\r\nContent-Length: "));
        char lenstr[256];
        snprintf(lenstr, 256, "%ld", (long) fileStats.st_size);
        write(fd, lenstr, strlen(lenstr));
        write(fd, "\r\n\r\n", 4);
        return 1;
    } else {
        perror("[response.c] stat");
        return 0;
    }
}

int writeError(int fd, int error, bool writebody) {
    char finalPath[4096];
    snprintf(finalPath, 4096, "%s/%d.html", ERROR_ROOT, error);

    if (isPathValid(finalPath, false)) {
        struct stat fileStats;
        if (stat(finalPath, &fileStats) == 0) {
            char headers[2048];
            
            int responsecount = sizeof(responses) / sizeof(responses[0]);
            for (int i = 0; i < responsecount; i++) {
                if (responses[i].code == error) {
                    snprintf(headers, 2048, "HTTP/1.1 %d %s\r\n"
                                    "Server: %s\r\n"
                                    "Content-type: text/html;charset=UTF-8\r\n"
                                    "Content-Length: %ld\r\n\r\n", error, responses[i].text, SERVER_BANNER, (long) fileStats.st_size);
                    write(fd, headers, strlen(headers));

                    if (writebody) {
                        int readbytes;
                        char fileReadBuf[16384];
                        int filefd = open(finalPath, O_RDONLY);

                        while ((readbytes = read(filefd, fileReadBuf, 16384)) > 0) {
                            write(fd, fileReadBuf, readbytes);
                        }
                    }
                    
                    return 1;
                }
            }
        } else {
            perror("[response.c] stat");
            return 0;
        }
    } else {
        char headers[2048];
        char body[2048];
        
        int responsecount = sizeof(responses) / sizeof(responses[0]);
        for (int i = 0; i < responsecount; i++) {
            if (responses[i].code == error) {
                snprintf(body, 2048, "<!DOCTYPE html>\r\n"
                                    "<html>\r\n"
                                    "<head>\r\n"
                                    "<title>%d %s</title>\r\n"
                                    "</head>\r\n"
                                    "<body>\r\n"
                                    "<h1>%d %s</h1>\r\n"
                                    "<hr>\r\n"
                                    "%s\n"
                                    "</body>\r\n"
                                    "</html>\r\n", error, responses[i].text, error, responses[i].text, SERVER_BANNER);
                
                snprintf(headers, 2048, "HTTP/1.1 %d %s\r\n"
                                        "Server: %s\r\n"
                                        "Content-type: text/html;charset=UTF-8\r\n"
                                        "Content-Length: %d\r\n\r\n", error, responses[i].text, SERVER_BANNER, strlen(body));
                write(fd, headers, strlen(headers));
                if (writebody) { write(fd, body, strlen(body)); }

                return 2;
            }
        }
    }

    return 0;
}

int writePointNineError(int fd, int error) {
    char errorString[256];
    snprintf(errorString, 256, "Error %d", error);
    write(fd, errorString, strlen(errorString));

    return 1;
}