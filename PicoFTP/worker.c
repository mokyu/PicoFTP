/*
 * The MIT License
 *
 * Copyright 2018 Mokyu.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <string.h>
#include "server.h"
#include "ftp.h"

void *clientHandler(void *args) {
    struct client_t *client = (struct client_t*) args;
    client->state = malloc(sizeof (state_t));
    client->state->loggedIn = 0;
    pthread_detach(pthread_self());
    printf("New connection accepted: %s:%u\n", inet_ntoa(client->addr.sin_addr), (unsigned int) ntohs(client->addr.sin_port));

    snprintf(client->outBuffer, BUFFER_SIZE, "220 Welcome to PicoFTP\r\n");
    write(client->fd, client->outBuffer, BUFFER_SIZE);
    while (1) {
        memset(client->inBuffer, 0, BUFFER_SIZE);
        memset(client->outBuffer, 0, BUFFER_SIZE);
        int bytesRead = read(client->fd, client->inBuffer, BUFFER_SIZE);

        if (bytesRead == 0) {
            printf("Connection Closed\n");
            break;
        }
        if (bytesRead < 0 && errno == EAGAIN) {
            printf("no data\n");
            continue;
        }
        if (bytesRead < 0) {
            printf("Connection error\n");
            break;
        }
        for (int i = 0; i < BUFFER_SIZE - 1; i++) {
            if (client->inBuffer[i] == '\n') {
                char *savePtr = NULL;
                char *token;
                for (token = strtok_r(client->inBuffer, "\n\r", &savePtr);
                        token != NULL;
                        token = strtok_r(NULL, "\n\r", &savePtr)) {
                    ftpCommands(client, token);
                }
                write(client->fd, client->outBuffer, BUFFER_SIZE);
                // write our result to the client
                break;
            } else {
                continue;
            }
        }
        memset(client->inBuffer, 0, BUFFER_SIZE);
        memset(client->outBuffer, 0, BUFFER_SIZE);
    }
    freeListener((listener_t*) client);
    pthread_exit(0);
}        
