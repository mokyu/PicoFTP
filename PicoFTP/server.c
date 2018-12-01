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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#include "server.h"
#include "config.h"
#include "worker.h"
struct listener_t* createListener(struct config_t* config, int isClient);
void freeListener(listener_t* listener);

void listener(struct config_t *config) {
    printf("Starting listener on port %u.\nUsing FTP root folder: %s\n", (unsigned int) config->port, config->path);
    struct listener_t* server = createListener(config, 0);
    pthread_t workerThread;

    // catch SIGPIPE on broken pipe if connection is lost.
    sigset_t signalToMask;
    sigemptyset(&signalToMask);
    sigaddset(&signalToMask, SIGPIPE);
    sigaddset(&signalToMask, SIGPIPE);
    if (pthread_sigmask(SIG_BLOCK, &signalToMask, NULL) != 0) {
        printf("Failed to mask SIGPIPE?\n");
        exit(errno);
    }
    if (bind(server->fd, (struct sockaddr*) &server->addr, sizeof (server->addr)) != 0) {
        printf("Failed to bind to specified socket. Are we trying to bind to a privileged port?\n");
        freeListener(server);
        exit(errno);
    }
    if (listen(server->fd, config->port) != 0) {
        printf("Failed to listen to socket. Are we trying to listen on a privileged port?\n");
        freeListener(server);
        exit(errno);
    };
    printf("Listening...\n");
    while (1) {
        struct client_t* client = (client_t*) createListener(config, 1);

        int len = sizeof (client->addr);

        if ((client->fd = accept(server->fd, (struct sockaddr*) &client->addr, &len)) == -1) {
            freeListener((listener_t*) client);
            continue;
        }

        if (pthread_create(&workerThread, NULL, clientHandler, client) < 0) {
            printf("Failed to create thread! Yikes..\n");
            freeListener((listener_t *) client);
            exit(errno);
        }
    }
    pthread_join(workerThread, NULL);
    pthread_exit(NULL);
    freeListener(server);
    return;
}

struct listener_t* createListener(struct config_t* config, int isClient) {
    struct listener_t *listener = malloc(sizeof (listener_t));
    memset(listener, 0, sizeof(listener_t));
    listener->bufferLen = BUFFER_SIZE + 1;
    listener->sockLen = sizeof (listener->addr);
    if (isClient) {
        return listener;
    }
    listener->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listener->fd == 0) {
        printf("Failed to create socket\n");
        free(listener->outBuffer);
        free(listener->inBuffer);
        free(listener);
        exit(errno);
    }
    listener->addr.sin_family = AF_INET;
    listener->addr.sin_port = htons(config->port);
    listener->addr.sin_addr.s_addr = INADDR_ANY;
    return listener;
}

void freeListener(listener_t* listener) {
    close(listener->fd);
    free(listener->state);
    free(listener);
    return;
}