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
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include "server.h"
#include "ftp.h"
#include <string.h>
#include "lookup.h"
#include "client.h"
#include "path.h"
#include <sys/sendfile.h>

typedef struct command_t {
    ftp_command command;
    char *argv[256];
    char fullarg[1024];
    int argc;
} command_t;

struct command_t* commandParser(char* token);

void bindDataPort(client_t *client);

void ftpCommands(client_t *client, char* token) {
    char* bufferOffset = &client->outBuffer[strlen(client->outBuffer)];
    int bufferMaxOffset = BUFFER_SIZE - strlen(client->outBuffer);
    char path[PATH_MAX];
    path_toString(client->state->path, path, COMPLETE);
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
                return;

        }
    }
    if (client->state->PASV) {
        switch (command->command) {
            case FTP_LIST_COMMAND:
                //
            {
                char* announce = "150 Here comes the directory listing\r\n";
                char* end = "226 Transfer complete\r\n";
                dirlist_t *listing = get_dirList(client);
                write(client->fd, announce, strlen(announce));
                write(client->state->clientFd, listing->list, strlen(listing->list));
                close(client->state->clientFd);
                client->state->PASVConnected = 0;
                client->state->PASV = 0;
                write(client->fd, end, strlen(end));
                free(listing);
                break;
            }
            case FTP_STOR_COMMAND:
            {
                if (command->argc > 0) {
                    // ignore everything and overwrite if it exists
                    char path[PATH_MAX];
                    if (command->fullarg[0] == '/') {
                        path_t* test = path_build(command->fullarg);
                        if (path_verify(client->state->path, test) == 1) { // our file path is legit, rejoice!
                            char* ready = "226 Ready\r\n";
                            write(client->fd, ready, strlen(ready));
                            char tmppath[PATH_MAX];
                            path_toString(client->state->path, tmppath, ROOTED);
                            snprintf(path, PATH_MAX - 1, "%s/%s", tmppath, command->fullarg + 1);
                        } else {
                            snprintf(bufferOffset, bufferMaxOffset, "500 Invalid filename\r\n");
                        }
                        path_free(test);
                    } else {
                        char tmppath[PATH_MAX];
                        path_toString(client->state->path, tmppath, COMPLETE);
                        snprintf(path, PATH_MAX - 1, "%s/%s", tmppath, command->fullarg);
                    }
                    printf("PATH TEST: %s\n", path);
                    FILE* fd = fopen(path, "w");
                    while (1) {
                        char buffer[4096];
                        int bytesRead = read(client->state->clientFd, buffer, 4096);
                        if (bytesRead == 0) {
                            printf("Connection Closed\n");
                            close(client->state->clientFd);
                            client->state->PASVConnected = 0;
                            client->state->PASV = 0;
                            break;
                        }
                        if (bytesRead < 0 && errno == EAGAIN) {
                            printf("empty buff??\n");
                            continue;
                        }
                        if (bytesRead < 0) {
                            printf("Transfer error\n");
                            snprintf(bufferOffset, bufferMaxOffset, "500 Transfer error\r\n");
                            break;
                        }
                        printf("b: %d\n", bytesRead);
                        fwrite(buffer, sizeof(char), bytesRead, fd);
                        //fseek(fd, bytesRead, SEEK_CUR);
                    }
                    fclose(fd);
                    close(client->state->clientFd);
                    client->state->PASVConnected = 0;
                    client->state->PASV = 0;
                    snprintf(bufferOffset, bufferMaxOffset, "226 File Transfer OK\r\n");

                } else {
                    snprintf(bufferOffset, bufferMaxOffset, "500 Missing parameter\r\n");
                }
                break;
            }
            case FTP_RETR_COMMAND:
                if (command->argc > 0) {
                    if (command->fullarg[0] == '/') { // is absolute path
                        path_t* test = path_build(command->fullarg);
                        if (path_verify(client->state->path, test) == 1) {
                            // the file is valid and within our root
                            char str1[PATH_MAX];
                            char str2[PATH_MAX];
                            path_toString(client->state->path, str1, ROOTED);
                            snprintf(str2, PATH_MAX - 1, "%s/%s", str1, command->fullarg);
                            FILE *file = fopen(str2, "r");
                            if (file) {
                                char* ready = "150 sending file\r\n";
                                write(client->fd, ready, strlen(ready));
                                char buffer[4096];
                                int bytesRead;
                                while ((bytesRead = fread(buffer, 1, sizeof (buffer), file)) > 0) {
                                    write(client->state->clientFd, buffer, 4096);
                                }
                                fclose(file);
                                close(client->state->clientFd);
                                client->state->PASVConnected = 0;
                                client->state->PASV = 0;
                                snprintf(bufferOffset, bufferMaxOffset, "226 File Transfer OK\r\n");
                            } else {
                                snprintf(bufferOffset, bufferMaxOffset, "500 file does not exist\r\n");
                            }
                            break;
                        }
                    } else {// relative path
                        char str[PATH_MAX];
                        path_toString(client->state->path, str, COMPLETE);
                        char str2[PATH_MAX];
                        if (strcmp(str, "/") == 0) {
                            str[0] = '\0';
                        }
                        snprintf(str2, PATH_MAX - 1, "%s/%s", str, command->fullarg);
                        FILE *file = fopen(str2, "r");
                        if (file) {
                            char* ready = "150 sending file\r\n";
                            write(client->fd, ready, strlen(ready));
                            char buffer[4096];
                            int bytesRead;
                            while ((bytesRead = fread(buffer, 1, sizeof (buffer), file)) > 0) {
                                write(client->state->clientFd, buffer, bytesRead);
                            }
                            fclose(file);
                            close(client->state->clientFd);
                            client->state->PASVConnected = 0;
                            client->state->PASV = 0;
                            snprintf(bufferOffset, bufferMaxOffset, "226 File Transfer OK\r\n");
                        } else {
                            snprintf(bufferOffset, bufferMaxOffset, "500 file does not exist\r\n");
                        }
                    }
                    // check if the file exists
                    // if the file exists send message to client
                    // else return error
                    // fill up buffer with file and send it until its sent complete
                    // close connection, boom


                } else {
                    snprintf(bufferOffset, bufferMaxOffset, "500 Missing parameter\r\n");
                }
                break;
            default:
                break;
        }
    }
    switch (command->command) {
        case FTP_PWD_COMMAND:
        {
            char path[PATH_MAX];
            path_toString(client->state->path, path, ROOTED);
            snprintf(bufferOffset, bufferMaxOffset, "257 \"%s\"\r\n", path);
            break;
        }
        case FTP_CWD_COMMAND:
        {
            if (command->argc > 0) {
                if (changeDir(client, command->fullarg)) {
                    snprintf(bufferOffset, bufferMaxOffset, "250 Okay\r\n");
                } else {
                    snprintf(bufferOffset, bufferMaxOffset, "550 Failed\r\n");
                }
            } else {
                snprintf(bufferOffset, bufferMaxOffset, "500 Missing parameter\r\n");
            }
            break;
        }
        case FTP_SYST_COMMAND:
            snprintf(bufferOffset, bufferMaxOffset - strlen(client->outBuffer), "215 UNIX Type: L8\r\n");
            break;
        case FTP_PASV_COMMAND:
            bindDataPort(client);
            snprintf(bufferOffset, bufferMaxOffset - strlen(client->outBuffer), "227 Entering Passive Mode (%s,%d,%d).\r\n", client->config->ip, client->state->port->portnum >> 8, client->state->port->portnum & 0x00FF);
            client->state->PASV = 1;
            break;
        case FTP_TYPE_COMMAND:
            if (command->argc > 0 && command->argv[1][0] == 'I') {
                client->state->transferMode = 'I';
                snprintf(bufferOffset, bufferMaxOffset, "200 Switching to Binary mode.\r\n");
            } else if (command->argc > 0 && command->argv[1][0] == 'A') {
                client->state->transferMode = 'A';
                snprintf(bufferOffset, bufferMaxOffset, "200 Switching to ASCII mode.\r\n");
            } else {
                snprintf(bufferOffset, bufferMaxOffset, "500 Missing parameter\r\n");
            }
            break;
        case FTP_MKD_COMMAND: // fix with path_verify()
            if (command->argc > 0) {
                if (command->argv[1][0] == '/') {
                    snprintf(bufferOffset, bufferMaxOffset, "500 Unable to create absolute directory\r\n");
                    break;
                }
                char path[PATH_MAX];
                path_toString(client->state->path, path, COMPLETE);
                if (strcmp(path, "/") == 0) {
                    path[0] = '\0';
                }
                snprintf(path + strlen(path), NAME_MAX - 1, "/%s", command->fullarg);
                if (mkdir(path, 0777) != -1) {
                    snprintf(bufferOffset, bufferMaxOffset, "257 Success.\r\n");
                } else {
                    snprintf(bufferOffset, bufferMaxOffset, "500 Unable to create.\r\n");
                }
            } else {
                snprintf(bufferOffset, bufferMaxOffset, "500 Missing parameter\r\n");
            }
            break;
        case FTP_RNFR_COMMAND:
            if (command->argc > 0) {
                if (command->fullarg[0] == '/') { // is absolute path
                    if (access(command->fullarg, F_OK) != -1) {
                        path_t* test = path_build(command->fullarg);
                        if (path_verify(client->state->path, test) == 1) {
                            // the file is valid and within our root
                            snprintf(bufferOffset, bufferMaxOffset, "350 Rename to?\r\n");
                            if (client->state->renameFrom != NULL) {
                                free(client->state->renameFrom);
                                client->state->renameFrom = NULL;
                            }
                            char* from = malloc(PATH_MAX);
                            snprintf(from, PATH_MAX - 1, "%s", command->fullarg);
                            break;
                        }
                    }
                } else {
                    char str[PATH_MAX];
                    path_toString(client->state->path, str, COMPLETE);
                    char str2[PATH_MAX];
                    if (strcmp(str, "/") == 0) {
                        str[0] = '\0';
                    }
                    snprintf(str2, PATH_MAX - 1, "%s/%s", str, command->fullarg);
                    if (access(str2, F_OK) != -1) {
                        snprintf(bufferOffset, bufferMaxOffset, "350 Rename to?\r\n");
                        if (client->state->renameFrom != NULL) {
                            free(client->state->renameFrom);
                            client->state->renameFrom = NULL;
                        }
                        client->state->renameFrom = malloc(PATH_MAX);
                        snprintf(client->state->renameFrom, PATH_MAX - 1, "%s", str2);
                        break;
                    }
                }
                snprintf(bufferOffset, bufferMaxOffset, "500 Invalid file\r\n");
            } else {
                snprintf(bufferOffset, bufferMaxOffset, "500 Missing parameter\r\n");
            }
            break;
        case FTP_RNTO_COMMAND:
            if (command->argc > 0 && client->state->renameFrom != NULL) {
                if (command->fullarg[0] == '/') { // is absolute path
                    path_t* test = path_build(command->fullarg);
                    if (path_verify(client->state->path, test) == 1) {
                        if (rename(client->state->renameFrom, command->fullarg) != 0) {
                            snprintf(bufferOffset, bufferMaxOffset, "500 Failed to rename file\r\n");
                        } else {
                            snprintf(bufferOffset, bufferMaxOffset, "250 Success\r\n");
                            free(client->state->renameFrom);
                            client->state->renameFrom = NULL;
                        }
                    }
                } else {
                    char str[PATH_MAX];
                    path_toString(client->state->path, str, COMPLETE);
                    if (strcmp(str, "/") == 0) {
                        str[0] = '\0';
                    }
                    char str2[PATH_MAX];
                    snprintf(str2, PATH_MAX - 1, "%s/%s", str, command->fullarg);
                    if (rename(client->state->renameFrom, str2) != 0) {
                        snprintf(bufferOffset, bufferMaxOffset, "500 Failed to rename file\r\n");
                    } else {
                        snprintf(bufferOffset, bufferMaxOffset, "250 Success\r\n");
                        free(client->state->renameFrom);
                        client->state->renameFrom = NULL;
                    }
                }

            } else {
                snprintf(bufferOffset, bufferMaxOffset, "500 Please use RNFR first or missing parameter\r\n");
            }
            break;
        case FTP_DELE_COMMAND:
            if (command->argc > 0) {
                char str[PATH_MAX];
                path_toString(client->state->path, str, COMPLETE);
                if (strcmp(str, "/") == 0) {
                    str[0] = '\0';
                }
                char str2[PATH_MAX];
                snprintf(str2, PATH_MAX - 1, "%s/%s", str, command->fullarg);
                if (access(str2, F_OK) != -1) {
                    if (remove(str2) == 0) {
                        snprintf(bufferOffset, bufferMaxOffset, "250 file deleted\r\n");
                    } else {
                        snprintf(bufferOffset, bufferMaxOffset, "450 Unable to comply, unknown error.\r\n");
                    }
                } else {
                    snprintf(bufferOffset, bufferMaxOffset, "450 Unable to comply, file does not exist.\r\n");
                }
            } else {
                snprintf(bufferOffset, bufferMaxOffset, "500 Missing argument\r\n");
            }
            break;
        case FTP_RMD_COMMAND:
            if (command->argc > 0) {
                char str[PATH_MAX];
                path_toString(client->state->path, str, COMPLETE);
                if (strcmp(str, "/") == 0) {
                    str[0] = '\0';
                }
                char str2[PATH_MAX];
                snprintf(str2, PATH_MAX - 1, "%s/%s", str, command->fullarg);
                if (access(str2, F_OK) != -1) {
                    if (rmdir(str2) == 0) {
                        snprintf(bufferOffset, bufferMaxOffset, "250 directory deleted\r\n");
                    } else {
                        snprintf(bufferOffset, bufferMaxOffset, "450 Unable to comply, unknown error.\r\n");
                    }
                } else {
                    snprintf(bufferOffset, bufferMaxOffset, "450 Unable to comply, Directory does not exist.\r\n");
                }
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

struct command_t * commandParser(char* responseToken) {
    command_t *command = malloc(sizeof (*command));
    command->command = lookupCommand(responseToken);
    command->argc = 0;
    int offset = 0;
    if (responseToken[3] == ' ') {
        offset = 4;
    } else {
        offset = 5;
    }
    snprintf(command->fullarg, 1023, "%s", responseToken + offset);
    char *savePtr = NULL;
    char *token = NULL;
    for (token = strtok_r(responseToken, " ", &savePtr);
            token != NULL;
            token = strtok_r(NULL, " ", &savePtr)) {
        command->argv[command->argc] = token;
        command->argc++;
    }
    return command;
}

void bindDataPort(client_t * client) {
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
        printf("Failed to bind to port: %d, retrying\n", port);
        close(conn->passiveFd);
        bindDataPort(client);
    } else if (listen(conn->passiveFd, 10) != 0) {
        bindDataPort(client);
    }
}