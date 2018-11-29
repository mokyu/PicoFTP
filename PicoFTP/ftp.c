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
#include <pthread.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include "server.h"
#include "ftp.h"
#include <string.h>
#include "lookup.h"

typedef struct command_t {
    ftp_command command;
    char *argv[32];
    int argc;
} command_t;

struct command_t* commandParser(client_t* client);

void ftpCommands(client_t *client) {
    command_t *command = commandParser(client);
    switch (command->command) {
        case FTP_AUTH_COMMAND:
            snprintf(client->outBuffer, BUFFER_SIZE, "500 Unsupported command\r\n");
            break;
        case FTP_USER_COMMAND:
            if(!strcmp(command->argv[1], "anonymous")) {
                snprintf(client->outBuffer, BUFFER_SIZE, "230 Welcome, anonymous user ♥\r\n");
            }else {
                snprintf(client->outBuffer, BUFFER_SIZE, "331 Password please :)\r\n");
            }
            break;
        case FTP_PASS_COMMAND:
            // password compare logic
            snprintf(client->outBuffer, BUFFER_SIZE, "230 welcome <username>!\r\n");
            break;
        case FTP_PWD_COMMAND:
            break;
        case FTP_UNKNOWN_COMMAND:
            break;
        default:
            break;
    }
    free(command);
}

struct command_t* commandParser(client_t* client) {
    command_t *command = malloc(sizeof (command_t));
    command->command = lookupCommand(client->inBuffer);
    command->argc = 0;
    char *token;
    while ((token = strtok_r(client->inBuffer, " \n\r", &client->inBuffer))) {
        command->argv[command->argc] = token;
        command->argc++;
        if (command->argc == 31) {
            printf("Could not parse more words from input buffer. Increase your buffer size.\n");
            break;
        }
    }
    return command;
}
