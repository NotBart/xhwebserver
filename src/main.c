/*
Filename: main.c
Author: Bartoloměj Bičovský
Purpose: main process
Version: 0.0.1
*/

#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sockets.h"
#include "request.h"

int main() {
    struct socketResult newSocket = createSocket(WEB_PORT);
    if (newSocket.socketfd < 0) {
        return EXIT_FAILURE;
    }
    int addrsize = sizeof(struct sockaddr_in);
    struct sockaddr_in6 boundAddress = newSocket.boundaddr;
    int inSocket = (int) newSocket.socketfd;

    while (1) {
        int reqs = accept(inSocket, (struct sockaddr *) &boundAddress, &addrsize);
        
        if (fork() == 0) {
            close(inSocket);
            handleRequest(reqs);
            exit(EXIT_SUCCESS);
        } else {
            close(reqs);
        }
    }

    close(inSocket);
    return EXIT_SUCCESS;
}