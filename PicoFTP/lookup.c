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
#include "lookup.h"
#include <string.h>
#include <stdlib.h>

cmd_t* lookupCommand(char* buffer) {
    char command[5];
    memcpy(&command, buffer, 4);
    command[4] = '\0';
    cmd_t *cmd = malloc(sizeof (arg_t));
    /*
    if (!strcmp(&command[0], "AUTH")) {
        return FTP_AUTH_COMMAND;
    }
     */
    if (!strcmp(&command[0], "PWD")) {
        cmd->func = &ftp_pwd_command;
        return cmd;
    }

    if (!strcmp(&command[0], "USER")) {
        cmd->func = &ftp_user_command;
        return cmd;
    }

    if (!strcmp(&command[0], "PASS")) {
        cmd->func = &ftp_pass_command;
        return cmd;
    }

    if (!strcmp(&command[0], "CWD ")) {
        cmd->func = &ftp_cwd_command;
        return cmd;
    }

    if (!strcmp(&command[0], "SYST")) {
        cmd->func = &ftp_syst_command;
        return cmd;
    }

    if (!strcmp(&command[0], "TYPE")) {
        cmd->func = &ftp_type_command;
        return cmd;
    }

    if (!strcmp(&command[0], "MKD ")) {
        cmd->func = &ftp_mkd_command;
        return cmd;
    }

    if (!strcmp(&command[0], "RMD ")) {
        cmd->func = &ftp_rmd_command;
        return cmd;
    }

    if (!strcmp(&command[0], "RNFR")) {
        cmd->func = &ftp_rnfr_command;
        return cmd;
    }
    
    if (!strcmp(&command[0], "RNTO")) {
        cmd->func = &ftp_rnto_command;
        return cmd;
    }
    
    if (!strcmp(&command[0], "DELE")) {
        cmd->func = &ftp_dele_command;
        return cmd;
    }

    if (!strcmp(&command[0], "PASV")) {
        cmd->func = &ftp_pasv_command;
        return cmd;
    }
    
    if (!strcmp(&command[0], "LIST")) {
        cmd->func = &ftp_list_command;
        return cmd;
    }
    
    if (!strcmp(&command[0], "STOR")) {
        cmd->func = &ftp_stor_command;
        return cmd;
    }
    
    if(!strcmp(&command[0], "RETR")) {
        cmd->func = &ftp_retr_command;
        return cmd;
    }

    cmd->func = &ftp_unknown_command;
    return cmd;
}