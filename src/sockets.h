/*
Filename: sockets.h
Author: Bartoloměj Bičovský
Purpose: socket header
Version: 0.0.1
*/

#ifndef SOCKETS_H
#define SOCKETS_H

#include <arpa/inet.h>

struct socketResult {
    int socketfd;
    struct sockaddr_in6 boundaddr;
};

struct socketResult createSocket(const unsigned short int port);

#endif