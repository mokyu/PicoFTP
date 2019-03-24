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
 * File:   ftp.h
 * Author: Mokyu
 *
 * Created on November 28, 2018, 3:03 PM
 */

#ifndef FTP_H
#define FTP_H

#ifdef __cplusplus
extern "C" {
#endif
#include <linux/limits.h>
#include "path.h"

    typedef struct arg_t {
        char *argv[256];
        char fullarg[1024];
        int argc;
    } arg_t;
    void ftp_unknown_command(client_t *client, arg_t *args);
    void ftp_user_command(client_t *client, arg_t *args);
    void ftp_pass_command(client_t *client, arg_t *args);
    void ftp_pwd_command(client_t *client, arg_t *args);
    void ftp_cwd_command(client_t *client, arg_t *args);
    void ftp_syst_command(client_t *client, arg_t *args);
    void ftp_type_command(client_t *client, arg_t *args);
    void ftp_mkd_command(client_t *client, arg_t *args);
    void ftp_rmd_command(client_t *client, arg_t *args);
    void ftp_rnfr_command(client_t *client, arg_t *args);
    void ftp_rnto_command(client_t *client, arg_t *args);
    void ftp_dele_command(client_t *client, arg_t *args);
    void ftp_pasv_command(client_t *client, arg_t *args);
    void ftp_list_command(client_t *client, arg_t *args);
    void ftp_stor_command(client_t *client, arg_t *args);
    void ftp_retr_command(client_t *client, arg_t *args);
    void handleCommand(client_t *client, char* token);

    typedef struct passive_t {
        int passiveFd;
        int portnum;
        struct sockaddr_in addr;
    } passive_t;

    typedef struct state_t {
        int PASV;
        int PASVConnected;
        int dataSocket;
        int loggedIn;
        char transferMode;
        char userName[32];
        struct passive_t* port;
        path_t* path;
        char* renameFrom;
        char* renameTo;
    } state_t;

#ifdef __cplusplus
}
#endif

#endif /* FTP_H */

