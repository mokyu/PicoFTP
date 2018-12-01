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

struct command_t* commandParser(char* token);

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
    } else {
        switch (command->command) {
            case FTP_PWD_COMMAND:
                snprintf(bufferOffset, bufferMaxOffset, "500 Unsupported command\r\n");
                break;
            case FTP_SYST_COMMAND:
                snprintf(bufferOffset, bufferMaxOffset - strlen(client->outBuffer), "215 UNIX type: L8\r\n");
                break;
            default:
                snprintf(bufferOffset, bufferMaxOffset, "500 unknown command\r\n");
                break;
        }
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
