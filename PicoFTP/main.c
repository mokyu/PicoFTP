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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "config.h"
#include "server.h"

void print_usage(void);

int main(int argc, char** argv) {
    struct config_t *config = malloc(sizeof(config_t));
    memset(config, 0, sizeof (config_t));
    config->port = 21;
    config->ftpRoot = "./";
    int opt;
    while ((opt = getopt(argc, argv, "p:d:a:")) != -1) {
        switch (opt) {
            case 'd':
                config->ftpRoot = optarg;
                break;
            case 'p':
                config->port = (unsigned short) atoi(optarg);
                break;
            case 'a':
                strncpy(config->ip, optarg, 15);
                break;
            default:
                print_usage();
                exit(EXIT_FAILURE);
        }
    }
    if(strlen(config->ip) <= 2) {
        print_usage();
        exit(EXIT_FAILURE);
    }
    listener(config);
    return (EXIT_SUCCESS);
}

void print_usage(void) {
    printf("Expected argument(s):\n-a <ip address> (Mandatory)\n-p <port number>\n-d <relative/full path to ftp root>\n");
}