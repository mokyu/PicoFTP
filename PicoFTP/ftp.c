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



struct arg_t* commandParser(char* token);

void bindDataPort(client_t *client);

void writeResponse(int fd, char* message) {
    write(fd, message, strlen(message));
}

void ftp_unknown_command(client_t *client, arg_t *args) {
    writeResponse(client->controlSocket, "500 Unknown or unsupported command\n");
}

void ftp_user_command(client_t *client, arg_t *args) {
    // we expect exactly 1 argument for the USER command
    if (args->argc == 2) {
        strncpy(client->state->userName, args->argv[1], 31);
        writeResponse(client->controlSocket, "331 Please supply password\n");
    } else {
        writeResponse(client->controlSocket, "500 Invalid parameter(s)\n");
    }
}

void ftp_pass_command(client_t *client, arg_t *args) {
    // As it is now the server accepts any combination of username and password.
    if (client->state->userName == NULL) {
        writeResponse(client->controlSocket, "500 Please provide username first\n");
        return;
    }
    if (args->argc == 2) {
        char resp[100];
        snprintf(resp, 99, "230 Welcome %s\n", client->state->userName);
        writeResponse(client->controlSocket, resp);
        client->state->loggedIn = 1;
    } else {
        writeResponse(client->controlSocket, "500 Invalid parameter(s)\n");
        client->state->loggedIn = 0;
    }
}

void ftp_pwd_command(client_t *client, arg_t *args) {
    if (client->state->loggedIn == 1) {
        char path[PATH_MAX];
        path_toString(client->state->path, path, ROOTED);
        char resp[PATH_MAX + 10];
        snprintf(resp, PATH_MAX + 9, "257 \"%s\"\n", path);
        writeResponse(client->controlSocket, resp);
    } else {
        writeResponse(client->controlSocket, "500 Please login first\n");
    }
}

void ftp_cwd_command(client_t *client, arg_t *args) {
    if (client->state->loggedIn == 1) {
        if (args->argc > 1) {
            if (changeDir(client, args->fullarg)) { // interpret the full list of args due to whitespace in folder names
                writeResponse(client->controlSocket, "250 Okay\n");
            } else {
                writeResponse(client->controlSocket, "550 Failed\n");
            }
        } else {
            writeResponse(client->controlSocket, "500 Invalid parameter(s).\n");
        }
    } else {
        writeResponse(client->controlSocket, "500 Please login first.\n");
    }
}

void ftp_syst_command(client_t *client, arg_t *args) {
    writeResponse(client->controlSocket, "215 UNIX Type: L8\n");
}

void ftp_type_command(client_t *client, arg_t *args) {
    if (client->state->loggedIn == 1) {
        if (args->argc == 2 && args->argv[1][0] == 'I') {
            writeResponse(client->controlSocket, "200 Switching to Binary mode.\n");
            // Does nothing
        } else if (args->argc == 2 && args->argv[1][0] == 'A') {
            writeResponse(client->controlSocket, "500 ASCII mode not supported. Only binary transfers.\n");
        } else {
            writeResponse(client->controlSocket, "500 Missing or unknown parameter(s).\n");
        }
    } else {
        writeResponse(client->controlSocket, "500 Please login first.\n");
    }
}

void ftp_mkd_command(client_t *client, arg_t *args) {
    if (client->state->loggedIn == 1) {
        if (args->argc > 0) {
            if (args->argv[1][0] == '/') {
                writeResponse(client->controlSocket, "500 Unable to create absolute directory\n");
                return;
            }
            char path[PATH_MAX];
            path_toString(client->state->path, path, COMPLETE);
            if (strcmp(path, "/") == 0) {
                path[0] = '\0';
            }
            snprintf(path + strlen(path), NAME_MAX - 1, "/%s", args->fullarg);
            if (mkdir(path, 0777) != -1) {
                writeResponse(client->controlSocket, "257 Success.\n");
            } else {
                writeResponse(client->controlSocket, "500 Unable to create.\n");
            }
        } else {
            writeResponse(client->controlSocket, "500 Missing parameter\n");
        }
    } else {
        writeResponse(client->controlSocket, "500 Please login first.\n");
    }
}

void ftp_rmd_command(client_t *client, arg_t *args) {
    if (client->state->loggedIn == 1) {
        if (args->argc > 0) {
            char str[PATH_MAX];
            path_toString(client->state->path, str, COMPLETE);
            if (strcmp(str, "/") == 0) {
                str[0] = '\0';
            }
            char str2[PATH_MAX];
            snprintf(str2, PATH_MAX - 1, "%s/%s", str, args->fullarg);
            if (access(str2, F_OK) != -1) {
                if (rmdir(str2) == 0) {
                    writeResponse(client->controlSocket, "250 directory deleted\n");
                } else {
                    writeResponse(client->controlSocket, "450 Unable to comply, unknown error.\n");
                }
            } else {
                writeResponse(client->controlSocket, "450 Unable to comply, Directory does not exist.\n");
            }
        }
    } else {
        writeResponse(client->controlSocket, "500 Please login first.\n");
    }
}

void ftp_rnfr_command(client_t *client, arg_t *args) {
    if (client->state->loggedIn == 1) {
        if (args->argc > 0) {
            if (args->fullarg[0] == '/') { // is absolute path
                if (access(args->fullarg, F_OK) != -1) {
                    path_t* test = path_build(args->fullarg);
                    if (path_verify(client->state->path, test) == 1) {
                        // the file is valid and within our root
                        writeResponse(client->controlSocket, "350 Rename to?\n");
                        if (client->state->renameFrom != NULL) {
                            free(client->state->renameFrom);
                            client->state->renameFrom = NULL;
                        }
                        char* from = malloc(PATH_MAX);
                        snprintf(from, PATH_MAX - 1, "%s", args->fullarg);
                        return;
                    }
                }
            } else {
                char str[PATH_MAX];
                path_toString(client->state->path, str, COMPLETE);
                char str2[PATH_MAX];
                if (strcmp(str, "/") == 0) {
                    str[0] = '\0';
                }
                snprintf(str2, PATH_MAX - 1, "%s/%s", str, args->fullarg);
                if (access(str2, F_OK) != -1) {
                    writeResponse(client->controlSocket, "350 Rename to?\n");
                    if (client->state->renameFrom != NULL) {
                        free(client->state->renameFrom);
                        client->state->renameFrom = NULL;
                    }
                    client->state->renameFrom = malloc(PATH_MAX);
                    snprintf(client->state->renameFrom, PATH_MAX - 1, "%s", str2);
                    return;
                }
            }
            writeResponse(client->controlSocket, "500 Invalid file.\n");
        } else {
            writeResponse(client->controlSocket, "500 Missing parameter.\n");
        }
    } else {
        writeResponse(client->controlSocket, "500 Please login first.\n");
    }
}

void ftp_rnto_command(client_t *client, arg_t *args) {
    if (client->state->loggedIn == 1) {
        if (args->argc > 0 && client->state->renameFrom != NULL) {
            if (args->fullarg[0] == '/') { // is absolute path
                path_t* test = path_build(args->fullarg);
                if (path_verify(client->state->path, test) == 1) {
                    if (rename(client->state->renameFrom, args->fullarg) != 0) {
                        writeResponse(client->controlSocket, "500 Failed to rename file\n");
                    } else {
                        writeResponse(client->controlSocket, "250 Success\n");
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
                snprintf(str2, PATH_MAX - 1, "%s/%s", str, args->fullarg);
                if (rename(client->state->renameFrom, str2) != 0) {
                    writeResponse(client->controlSocket, "500 Failed to rename file\n");
                } else {
                    writeResponse(client->controlSocket, "250 Success\n");
                    free(client->state->renameFrom);
                    client->state->renameFrom = NULL;
                }
            }

        } else {
            writeResponse(client->controlSocket, "500 Please use RNFR first or missing parameter\n");
        }
    } else {
        writeResponse(client->controlSocket, "500 Please login first.\n");
    }
}

void ftp_dele_command(client_t *client, arg_t *args) {
    if (client->state->loggedIn == 1) {
        if (args->argc > 0) {
            char str[PATH_MAX];
            path_toString(client->state->path, str, COMPLETE);
            if (strcmp(str, "/") == 0) {
                str[0] = '\0';
            }
            char str2[PATH_MAX];
            snprintf(str2, PATH_MAX - 1, "%s/%s", str, args->fullarg);
            if (access(str2, F_OK) != -1) {
                if (remove(str2) == 0) {
                    writeResponse(client->controlSocket, "250 file deleted\n");
                } else {
                    writeResponse(client->controlSocket, "450 Unable to comply, unknown error.\n");
                }
            } else {
                writeResponse(client->controlSocket, "450 Unable to comply, file does not exist.\n");
            }
        } else {
            writeResponse(client->controlSocket, "500 Missing argument\n");
        }
    } else {
        writeResponse(client->controlSocket, "500 Please login first.\n");
    }
}

void ftp_pasv_command(client_t *client, arg_t *args) {
    if (client->state->loggedIn == 1) {
        bindDataPort(client);
        char resp[255];
        snprintf(resp, 255, "227 Entering Passive Mode (%s,%d,%d).\n", client->config->ip, client->state->port->portnum >> 8, client->state->port->portnum & 0x00FF);
        writeResponse(client->controlSocket, resp);
        client->state->PASV = 1;
    } else {
        writeResponse(client->controlSocket, "500 Please login first.\n");
    }
}

void ftp_list_command(client_t *client, arg_t *args) {
    if (client->state->PASV == 1) {
        dirlist_t *listing = get_dirList(client);
        writeResponse(client->controlSocket, "150 Here comes the directory listing\n");
        // close data socket and destroy PASV session directly after sending data stream
        writeResponse(client->state->dataSocket, listing->list);
        close(client->state->dataSocket);
        client->state->PASVConnected = 0;
        client->state->PASV = 0;
        writeResponse(client->controlSocket, "226 Transfer complete\n");
        free(listing);
    } else {
        writeResponse(client->controlSocket, "500 Please use PASV first.\n");
    }
}

void ftp_stor_command(client_t *client, arg_t *args) {
    if (client->state->PASV == 1) {
        if (args->argc > 0) {
            // ignore everything and overwrite if it exists
            char path[PATH_MAX];
            if (args->fullarg[0] == '/') {
                path_t* test = path_build(args->fullarg);
                if (path_verify(client->state->path, test) == 1) { // our file path is legit, rejoice!
                    writeResponse(client->controlSocket, "226 Ready\n");
                    char tmppath[PATH_MAX];
                    path_toString(client->state->path, tmppath, ROOTED);
                    snprintf(path, PATH_MAX - 1, "%s/%s", tmppath, args->fullarg + 1);
                } else {
                    writeResponse(client->controlSocket, "500 Invalid filename\n");
                }
                path_free(test);
            } else {
                char tmppath[PATH_MAX];
                path_toString(client->state->path, tmppath, COMPLETE);
                snprintf(path, PATH_MAX - 1, "%s/%s", tmppath, args->fullarg);
            }
            FILE* fd = fopen(path, "w");
            while (1) {
                char buffer[4096];
                int bytesRead = read(client->state->dataSocket, buffer, 4096);
                if (bytesRead == 0) {
                    printf("Data Connection Closed\n");
                    close(client->state->dataSocket);
                    client->state->PASVConnected = 0;
                    client->state->PASV = 0;
                    break;
                }
                if (bytesRead < 0 && errno == EAGAIN) {
                    continue;
                }
                if (bytesRead < 0) {
                    printf("Transfer error\n");
                    writeResponse(client->controlSocket, "500 Transfer error\n");
                    break;
                }
                fwrite(buffer, sizeof (char), bytesRead, fd);
            }
            fclose(fd);
            close(client->state->dataSocket);
            client->state->PASVConnected = 0;
            client->state->PASV = 0;
            writeResponse(client->controlSocket, "226 File Transfer OK\n");

        } else {
            writeResponse(client->controlSocket, "500 Missing parameter\n");
        }
    } else {
        writeResponse(client->controlSocket, "500 Please use PASV first.\n");
    }
}

void ftp_retr_command(client_t *client, arg_t *args) {
    if (client->state->PASV == 1) {
        if (args->argc > 0) {
            if (args->fullarg[0] == '/') { // is absolute path
                path_t* test = path_build(args->fullarg);
                if (path_verify(client->state->path, test) == 1) {
                    // the file is valid and within our root
                    char str1[PATH_MAX];
                    char str2[PATH_MAX];
                    path_toString(client->state->path, str1, ROOTED);
                    snprintf(str2, PATH_MAX - 1, "%s/%s", str1, args->fullarg);
                    FILE *file = fopen(str2, "r");
                    if (file) {
                        writeResponse(client->controlSocket, "150 sending file\n");
                        char buffer[4096];
                        int bytesRead;
                        while ((bytesRead = fread(buffer, 1, sizeof (buffer), file)) > 0) {
                            write(client->state->dataSocket, buffer, 4096);
                        }
                        fclose(file);
                        close(client->state->dataSocket);
                        client->state->PASVConnected = 0;
                        client->state->PASV = 0;
                        writeResponse(client->controlSocket, "226 File Transfer OK\n");
                    } else {
                        writeResponse(client->controlSocket, "500 file does not exist\n");
                    }
                    return;
                }
            } else {// relative path
                char str[PATH_MAX];
                path_toString(client->state->path, str, COMPLETE);
                char str2[PATH_MAX];
                if (strcmp(str, "/") == 0) {
                    str[0] = '\0';
                }
                snprintf(str2, PATH_MAX - 1, "%s/%s", str, args->fullarg);
                FILE *file = fopen(str2, "r");
                if (file) {
                    writeResponse(client->controlSocket, "150 sending file\n");
                    char buffer[4096];
                    int bytesRead;
                    while ((bytesRead = fread(buffer, 1, sizeof (buffer), file)) > 0) {
                        write(client->state->dataSocket, buffer, bytesRead);
                    }
                    fclose(file);
                    close(client->state->dataSocket);
                    client->state->PASVConnected = 0;
                    client->state->PASV = 0;
                    writeResponse(client->controlSocket, "226 File Transfer OK\n");
                } else {
                    writeResponse(client->controlSocket, "500 file does not exist\n");
                }
            }

        } else {
            writeResponse(client->controlSocket, "500 Missing parameter\n");
        }
    } else {
        writeResponse(client->controlSocket, "500 Please use PASV first.\n");
    }
}

void handleCommand(client_t *client, char* token) {
    // fetch command from raw input string
    cmd_t* command = lookupCommand(token);
    // tokenize raw input string for ease of use.
    arg_t *args = commandParser(token);

    command->func(client, args);
    free(command);
    free(args);
}

struct arg_t * commandParser(char* responseToken) {
    arg_t *command = malloc(sizeof (*command));
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
