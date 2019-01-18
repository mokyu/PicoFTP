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
#include "server.h"
#include "ftp.h"
#include <string.h>

ftp_command lookupCommand(char* buffer) {
    char command[5];
    memcpy(&command, buffer, 4);
    command[4] = '\0';
    if (!strcmp(&command[0], "AUTH")) {
        return FTP_AUTH_COMMAND;
    }
    if (!strcmp(&command[0], "PWD")) {
        return FTP_PWD_COMMAND;
    }
    if (!strcmp(&command[0], "USER")) {
        return FTP_USER_COMMAND;
    }
    if (!strcmp(&command[0], "PASS")) {
        return FTP_PASS_COMMAND;
    }
    if (!strcmp(&command[0], "SYST")) {
        return FTP_SYST_COMMAND;
    }
    if (!strcmp(&command[0], "PASV")) {
        return FTP_PASV_COMMAND;
    }
    if (!strcmp(&command[0], "LIST")) {
        return FTP_LIST_COMMAND;
    }
    if (!strcmp(&command[0], "TYPE")) {
        return FTP_TYPE_COMMAND;
    }
    if (!strcmp(&command[0], "CWD ")) {
        return FTP_CWD_COMMAND;
    }if (!strcmp(&command[0], "MKD ")) {
        return FTP_MKD_COMMAND;
    }if (!strcmp(&command[0], "RNFR")) {
        return FTP_RNFR_COMMAND;
    }if (!strcmp(&command[0], "RNTO")) {
        return FTP_RNTO_COMMAND;
    }
    return FTP_UNKNOWN_COMMAND;
}