/*
Filename: urlencode.c
Author: Bartoloměj Bičovský
Purpose: decode url
Version: 0.0.1
*/

#include "urlencode.h"
#include "options.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

char *urlDecode(char *inbuf, bool urlsafe) {
    static char outbuf[MAX_URL_PATH*3+1];
    memset(outbuf, '\0', MAX_URL_PATH*3+1);

    int j = 0;
    for (int i = 0; i < strlen(inbuf); i++) {
        switch (inbuf[i]) {
            case '+':
                outbuf[j++] = ' ';
                break;

            case '%':
                if (i + 1 < strlen(inbuf) && i + 2 < strlen(inbuf) && isxdigit(inbuf[i+1]) && isxdigit(inbuf[i+2])) {
                    char encodedChar[3];
                    encodedChar[0] = inbuf[i+1];
                    encodedChar[1] = inbuf[i+2];
                    encodedChar[2] = '\0';
                    char decodedChar = (char) strtol(encodedChar, NULL, 16);
                    if (urlsafe) {
                        switch (decodedChar) {
                            case '/':
                            case '\0':
                            case '.':
                            case '*':
                            case ':':
                                strcpy(outbuf, "INVALID!!!!");
                                return outbuf;
                            
                            default:
                                break;
                        }
                    }
                    outbuf[j++] = decodedChar;
                    i += 2;
                } else {
                    // printf("%d %d %d %d", i + 1 < strlen(inbuf), i + 2 < strlen(inbuf), isxdigit(inbuf[i+1]), isxdigit(inbuf[i+2]));isxdigit(inbuf[i+2]);
                    strcpy(outbuf, "INVALID!!!!");
                    return outbuf;
                }
                break;

            default:
                outbuf[j++] = inbuf[i];
                break;
        }
    }

    outbuf[j] = '\0';
    return outbuf;
}