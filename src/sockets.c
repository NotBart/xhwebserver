/*
Filename: sockets.c
Author: Bartoloměj Bičovský
Purpose: create socket
Version: 0.0.1
*/

#include "sockets.h"
#include "options.h"
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

struct socketResult createSocket(const unsigned short int port) {
    printf("Creating port %d", port);
    int acceptingSocket = -1;
    struct socketResult returnValues;

    if ((acceptingSocket = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        printf("...error\n");
        perror("[sockets.c] socket");
        returnValues.socketfd = -10001;
        return returnValues;
    }
    
    returnValues.socketfd = acceptingSocket;

    struct sockaddr_in6 serverAddress;
    serverAddress.sin6_family = AF_INET6;
    serverAddress.sin6_addr = in6addr_any;
    serverAddress.sin6_port = htons(port);

    returnValues.boundaddr = serverAddress;

    if (bind(acceptingSocket, (struct sockaddr *) &serverAddress, sizeof(struct sockaddr_in6)) < 0) {
        printf("...error\n");
        perror("[sockets.c] bind");
        returnValues.socketfd = -10002;
        return returnValues;
    }

    if (listen(acceptingSocket, 128) < 0) {
        printf("...error\n");
        perror("[sockets.c] listen");
        returnValues.socketfd = -10003;
        return returnValues;
    }

    printf("...ok\n");
    return returnValues;
}