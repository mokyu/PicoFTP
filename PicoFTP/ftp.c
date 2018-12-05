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
#include <time.h>
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

struct command_t* commandParser(char* token);

void bindDataPort(client_t *client);

void ftpCommands(client_t *client, char* token) {
    char* bufferOffset = &client->outBuffer[strlen(client->outBuffer)];
    int bufferMaxOffset = BUFFER_SIZE - strlen(client->outBuffer);

    command_t *command = commandParser(token);
    if (!client->state->loggedIn) {
        switch (command->command) {
            case FTP_AUTH_COMMAND:
                snprintf(bufferOffset, bufferMaxOffset, "500 Unsupported command\r\n");
                break;
            case FTP_USER_COMMAND:
                strncpy(client->state->userName, command->argv[1], 32);
                snprintf(bufferOffset, bufferMaxOffset, "331 Password please :)\r\n");
                break;
            case FTP_PASS_COMMAND:
                // password compare logic
                snprintf(bufferOffset, bufferMaxOffset, "230 welcome %s!\r\n", client->state->userName);
                client->state->loggedIn = 1;
                break;
            default:
                snprintf(bufferOffset, bufferMaxOffset, "530 Please login first\r\n");
                break;

        }
    }
    if (client->state->PASV) {
        switch (command->command) {
            case FTP_LIST_COMMAND:
                printf("we got a list\n");
                break;
            default:
                break;
        }
    }
    switch (command->command) {
        case FTP_PWD_COMMAND:
            snprintf(bufferOffset, bufferMaxOffset, "257 \"/\"\r\n");
            break;
        case FTP_SYST_COMMAND:
            snprintf(bufferOffset, bufferMaxOffset - strlen(client->outBuffer), "215 UNIX type: L8\r\n");
            break;
        case FTP_PASV_COMMAND:
            bindDataPort(client);
            snprintf(bufferOffset, bufferMaxOffset - strlen(client->outBuffer), "227 Entering Passive Mode (%s,%d,%d).\r\n", client->config->ip, client->state->port->portnum >> 8, client->state->port->portnum & 0x00FF);
            client->state->PASV = 1;
            break;
        case FTP_TYPE_COMMAND:
            if (command->argv[1][0] == 'I') {
                client->state->transferMode = 'I';
                snprintf(bufferOffset, bufferMaxOffset, "200 Switching to Binary mode.\r\n");
            } else if (command->argv[1][0] == 'A') {
                client->state->transferMode = 'A';
                snprintf(bufferOffset, bufferMaxOffset, "200 Switching to ASCII mode.\r\n");
            } else {
                snprintf(bufferOffset, bufferMaxOffset, "500 Unsupported command\r\n");
            }
            break;
        default:
            break;
    }

    if (strlen(client->outBuffer) == 0) {
        snprintf(bufferOffset, bufferMaxOffset, "500 Unknown command\r\n");
    }
    free(command);
}

struct command_t* commandParser(char* responseToken) {
    command_t *command = malloc(sizeof (*command));
    command->command = lookupCommand(responseToken);
    command->argc = 0;
    char *savePtr = NULL;
    char *token;
    for (token = strtok_r(responseToken, " ", &savePtr);
            token != NULL;
            token = strtok_r(NULL, " ", &savePtr)) {
        command->argv[command->argc] = token;
        command->argc++;
    }
    return command;
}

void bindDataPort(client_t *client) {
    srand(time(NULL));
    int port = rand() % (0xFFFF - 32768) + 32768;
    printf("port: %d\n", port);
    // free old pointer in case we had one.
    free(client->state->port);
    client->state->port = malloc(sizeof (passive_t));
    passive_t *conn = client->state->port;
    conn->portnum = port;
    conn->passiveFd = socket(AF_INET, SOCK_STREAM, 0);
    conn->addr.sin_family = AF_INET;
    conn->addr.sin_addr.s_addr = INADDR_ANY;
    conn->addr.sin_port = htons(port);
    if (bind(conn->passiveFd, (struct sockaddr*) &conn->addr, sizeof (conn->addr)) != 0) {
        printf("Failed to bind to port...\n");
    }
    if (listen(conn->passiveFd, 10) != 0) {
        //failed to get port.. arf.
        bindDataPort(client);
    }
}