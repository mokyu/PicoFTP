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

/* 
 * File:   server.h
 * Author: Mokyu
 *
 * Created on November 27, 2018, 10:10 PM
 */

#ifndef SERVER_H
#define SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"
#include <sys/socket.h>
#include <arpa/inet.h>

    
#define BUFFER_SIZE 2047
    typedef struct listener_t {
        int fd; // Socket file description
        struct sockaddr_in addr;
        char outBuffer[BUFFER_SIZE];
        char inBuffer[BUFFER_SIZE];
        int sockLen;
        int bufferLen;
        struct state_t *state;
    } listener_t;

    typedef struct client_t {
        int fd; // Socket file description
        struct sockaddr_in addr;
        char outBuffer[BUFFER_SIZE];
        char inBuffer[BUFFER_SIZE];
        int sockLen;
        int bufferLen;
        struct state_t *state;
    } client_t;

    void listener(struct config_t *config);
    void freeListener(listener_t* listener);

#ifdef __cplusplus
}
#endif

#endif /* SERVER_H */

