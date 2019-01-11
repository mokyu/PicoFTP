/*
 * The MIT License
 *
 * Copyright 2018 parallels.
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
 * File:   client.h
 * Author: parallels
 *
 * Created on December 14, 2018, 2:12 PM
 */

#ifndef CLIENT_H
#define CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif
#include "server.h"
#include <dirent.h>
#include <sys/stat.h>
#include <linux/limits.h>
    typedef struct {
        char fullPath[PATH_MAX + 1];
        char ftpPath[PATH_MAX + 1];
    } directory_t;

    typedef struct {
        char list[0xFFFF];
    } dirlist_t;

    int changeDir(client_t* client, char* folder);
    directory_t* getDir(client_t* client);
    dirlist_t* get_dirList(client_t* client);
    void createDirEntry(struct stat* file, struct dirent* dirEntry, char* buffer);
    int leaveDir(client_t* client);
#ifdef __cplusplus
}
#endif

#endif /* CLIENT_H */

