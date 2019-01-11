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
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include "server.h"
#include "client.h"
#include "ftp.h"

char* perms(struct stat *st);
// enter a directory

int changeDir(client_t* client, char* folder) {
    return path_cwd(client->state->path, folder);
}

void createDirEntry(struct stat* file, struct dirent * dirEntry, char* buffer) {
    snprintf(buffer, 512, "%s 1 %d %d %ld %s %s\n", perms(file), file->st_uid, file->st_gid, file->st_size, "Jan 1 1970", dirEntry->d_name);
    return;
}

char* perms(struct stat * st) {
    char *modeval = malloc(sizeof (char) * 10 + 1);
    mode_t perm = st->st_mode;
    modeval[0] = (S_ISDIR(perm)) ? 'd' : '-';
    modeval[1] = (perm & S_IRUSR) ? 'r' : '-';
    modeval[1] = (perm & S_IRUSR) ? 'r' : '-';
    modeval[2] = (perm & S_IWUSR) ? 'w' : '-';
    modeval[3] = (perm & S_IXUSR) ? 'x' : '-';
    modeval[4] = (perm & S_IRGRP) ? 'r' : '-';
    modeval[5] = (perm & S_IWGRP) ? 'w' : '-';
    modeval[6] = (perm & S_IXGRP) ? 'x' : '-';
    modeval[7] = (perm & S_IROTH) ? 'r' : '-';
    modeval[8] = (perm & S_IWOTH) ? 'w' : '-';
    modeval[9] = (perm & S_IXOTH) ? 'x' : '-';
    modeval[10] = '\0';
    return modeval;
}

dirlist_t * get_dirList(client_t* client) {
    dirlist_t *entry = malloc(sizeof (dirlist_t));
    struct dirent *ent;
    DIR *dfd;
    char path[PATH_MAX];
    path_toString(client->state->path, path, COMPLETE);
    if ((dfd = opendir(path)) == NULL) {
        printf("Failed to open directory %s\n", path);
        closedir(dfd);
        return entry;
    }
    char fname[PATH_MAX + NAME_MAX];
    int offset = 0;
    while ((ent = readdir(dfd)) != NULL) {
        struct stat stbuf;
        snprintf(fname, 255, "%s/%s", path, ent->d_name);
        if (stat(fname, &stbuf) == -1) {
            printf("Failed to stat file %s, ignoring\n", ent->d_name);
            continue;
        }
        if(strcmp(ent->d_name, "..") == 0 && path_getLast(client->state->path)->rootFolder == 1) {
            continue; // ignore the .. folder when we are in the ftp root
        }
        char buffer[513];
        createDirEntry(&stbuf, ent, buffer);
        offset += snprintf(entry->list + offset, 0xFFFE, "%s", buffer);
    }
    
    //snprintf(entry->list, 0xFFFE, "%s", "input");
    closedir(dfd);
    return entry;
}