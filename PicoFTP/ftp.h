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
    void ftpCommands(client_t *client);

    typedef struct passive_t {
        int fd;
        struct sockaddr_in addr;
    } passive_t;

    typedef struct ftpState_t {
        struct passive_t* ports;
        char working_dir[255];
    } ftpState_t;

    typedef enum {
        FTP_AUTH_COMMAND,
        FTP_USER_COMMAND,
        FTP_PWD_COMMAND,
        FTP_UNKNOWN_COMMAND,
        FTP_PASS_COMMAND,
    } ftp_command;

#define FTP_SYNTAX_ERROR_500 "500 Syntax error, command unrecognized\r\n"
#define FTP_LOGIN_SUCCESS "230 Welcome, "

#ifdef __cplusplus
}
#endif

#endif /* FTP_H */

