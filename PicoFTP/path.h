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

/* 
 * File:   path.h
 * Author: parallels
 *
 * Created on January 6, 2019, 7:51 PM
 */

#ifndef PATH_H
#define PATH_H
#include <linux/limits.h>

#ifdef __cplusplus
extern "C" {
#endif
    // FORWARD -> create a string with the current item as root
    // BACKWARD -> create a string with the current item functioning as the limiter
    // COMPLETE -> convert the whole list to a path regardless of where it starts or ends

    typedef enum method_e {
        FORWARD,
        BACKWARD,
        COMPLETE,
        ROOTED
    } method_e;

    typedef struct path_t {
        struct path_t *up;
        struct path_t *down;
        int rootFolder;
        char name[NAME_MAX + 1];
    } path_t;

    int path_verify(path_t* source, path_t* target);
    void path_toString(path_t* link, char* buffer, method_e method);
    void path_free(path_t* link);
    path_t* path_build(char* path);
    int path_cwd(path_t* path, char* folder);
    path_t* path_getRoot(path_t* path);
    path_t* path_getLast(path_t* path);
#ifdef __cplusplus
}
#endif

#endif /* PATH_H */

