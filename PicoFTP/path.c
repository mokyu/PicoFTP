/*
 * The MIT License
 *
 * Copyright 2019 parallels.
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
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "path.h"

int path_shrink(path_t* path);
int path_extend(path_t* path, char* addpath);
path_t* path_getFirst(path_t* path);
path_t* path_getRoot(path_t* path);
path_t* path_getLast(path_t* path);
// build a path string from a path link

void path_toString(path_t* link, char* buffer, method_e method) {
    path_t* tag = NULL;
    int offset = 0;
    switch (method) {
        case BACKWARD:
        {
            tag = link;
            tag = path_getFirst(tag);

            do {
                offset += snprintf(buffer + offset, PATH_MAX, "/%s", tag->name);
                if (link == tag) {
                    break;
                }
                tag = tag->down;
            } while (tag != NULL); // forward through stack until we reach our target link and include the link
        }
            break;
        case FORWARD:
        {
            tag = link;
            do {

                offset += snprintf(buffer + offset, PATH_MAX, "/%s", tag->name);
                tag = tag->down;
            } while (tag != NULL);
        }
            break;
        case COMPLETE:
        {
            tag = link;
            tag = path_getFirst(tag);
            do {
                offset += snprintf(buffer + offset, PATH_MAX, "/%s", tag->name);
                tag = tag->down;
            } while (tag != NULL);
        }
            break;
        case ROOTED:
        {
            tag = path_getFirst(link);
            int rooted = 0;
            do {
                if (tag->rootFolder && rooted == 0) {
                    rooted = 1;
                    offset += snprintf(buffer + offset, PATH_MAX, "/"); // provide the root "/"
                } else if (rooted && tag->down == NULL) {
                    offset += snprintf(buffer + offset, PATH_MAX, "%s", tag->name); // if the last item in list don't follow up with a "/"
                } else if (rooted && tag->down != NULL) {
                    offset += snprintf(buffer + offset, PATH_MAX, "%s/", tag->name); // if not last in the list
                }
                tag = tag->down;
            } while (tag != NULL);
        }
            break;
    }
}

void path_free(path_t* link) {
    path_t* tag = link;
    path_t* old = NULL;
    tag = path_getFirst(tag);

    do {
        old = tag;
        tag = tag->down;
        free(old);
    } while (tag != NULL);
}

path_t* path_build(char* path) {
    char str[PATH_MAX];
    snprintf(str, PATH_MAX -1, "%s", path);
    // returns the last link of the path chain
    // sets last link of path to "root"
    path_t* currentdir = NULL;
    char *saved = NULL;
    char *token;
    for (token = strtok_r(str, "/", &saved); token; token = strtok_r(NULL, "/", &saved)) {
        if (currentdir == NULL) {
            currentdir = malloc(sizeof (path_t));
            snprintf(currentdir->name, NAME_MAX, "%s", token);
            currentdir->up = NULL;
        } else {
            path_t* updir = currentdir;
            currentdir = malloc(sizeof (path_t));
            currentdir->up = updir;
            updir->down = currentdir;
            snprintf(currentdir->name, NAME_MAX, "%s", token);
        }
    }
    if (currentdir != NULL) {
        currentdir->rootFolder = 1;
        currentdir->down = NULL;
    }
    return currentdir;
}

int path_cwd(path_t* path, char* change) {
    DIR *dfd;
    path_t* tag = path;
    char rootFolder[PATH_MAX + 1];
    path_toString(tag, rootFolder, BACKWARD);
    char newFolder[PATH_MAX + 1];
    if (change[0] == '/') {
        tag = path_getRoot(tag);
        snprintf(newFolder, PATH_MAX, "%s%s", rootFolder, change);
        if ((dfd = opendir(newFolder)) == NULL) {
            closedir(dfd);
            return 0;

        }
        // our path is valid, lets clean up our old path!
        while (path_shrink(path));
        // .. and extend it
        path_extend(path, change);
        return 1;
    } else {
        char pathstr[PATH_MAX + 1];
        path_toString(path, pathstr, COMPLETE);
        snprintf(newFolder, PATH_MAX, "%s/%s", pathstr, change);
        if ((dfd = opendir(newFolder)) == NULL) {
            closedir(dfd);
            return 0;
        }
        if (path_extend(path, change)) {
            char data[PATH_MAX];
            path_toString(path, data, COMPLETE);
            closedir(dfd);
            return 1;
        }
        return 0;
    }
}

int path_extend(path_t* path, char* addpath) {
    char *saved = NULL;
    char *token;
    path_t* tag = NULL;
    path_t* last = path_getLast(path);
    char newpath[PATH_MAX + 1];
    snprintf(newpath, PATH_MAX, "%s", addpath);
    for (token = strtok_r(newpath, "/", &saved); token; token = strtok_r(NULL, "/", &saved)) {
        if (strcmp(token, "..") == 0) {
            if (path_shrink(path) == 0) {
                printf("up folder\n");
                return 0;
            }
            tag = path_getLast(path);
            continue;
        } else if (strcmp(token, ".") == 0) {
            printf("current folder\n");
            continue;
        }
        if (tag == NULL) {
            tag = malloc(sizeof (path_t));
            snprintf(tag->name, NAME_MAX, "%s", token);
            tag->rootFolder = 0;
            tag->up = last;
            last->down = tag;
        } else {
            path_t* updir = tag;
            tag = malloc(sizeof (path_t));
            tag->up = updir;
            tag->rootFolder = 0;
            updir->down = tag;
            tag->down = NULL;
            snprintf(tag->name, NAME_MAX, "%s", token);
        }
    }
    if (tag != NULL) {
        tag->down = NULL;
    }
    return 1;
}

int path_shrink(path_t* path) { // shrinks path by 1, returns 0 when it can't (root folder)
    path_t* tag = path_getLast(path);
    path_t* up = tag->up;
    if (tag->rootFolder == 0) {
        free(tag);
        up->down = NULL;
        return 1;
    }
    return 0;
}

path_t* path_getLast(path_t* path) { // go furthest *down* the directory
    path_t* tag = path;
    while (tag->down != NULL) {
        tag = tag->down;
    }
    return tag;
}

path_t* path_getFirst(path_t* path) { // go furthest *up* the directory
    path_t* tag = path;
    while (tag->up != NULL) {
        tag = tag->up;
    }
    return tag;
}

path_t* path_getRoot(path_t* path) { // go up and then go down till we find the root directory
    path_t* tag = path_getFirst(path);
    while (tag->down != NULL) {
        if (tag->rootFolder != 1) {
            tag = tag->down;
        } else {
            break;
        }
    }
    return tag;
}

int path_verify(path_t* source, path_t* target) { // compare two paths, return 1 when the path is inside our root
    path_t* s = path_getFirst(source);
    path_t* t = path_getFirst(target);
    int result = 0;
    while (s != NULL && t != NULL) {
        if (s->rootFolder == 1) {
            if (strcmp(s->name, t->name) == 0) {
                result = 1;
            }
        }

    }
    return result;
}