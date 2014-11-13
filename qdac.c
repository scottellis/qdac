/*
 * Copyright (c) 2014 Jumpnow Technologies, LLC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "mcp4728-qdac.h"

void usage(char *argv_0)
{
    printf("\nUsage: %s [-rweh][<ch> [<val>]]\n", argv_0);
    printf("  <none>            Dump the output of all input and eeprom registers\n");
    printf("  -r <ch>           Read register for channel, <ch> range 0-3\n");
    printf("  -w <ch> <val>     Write register for channel, <ch> range 0-3, <val> range 0-4095\n");
    printf("                    Note: A value of zero will also disable that channel\n");
    printf("  -e                Use with -r or -w. Works on the eeprom reg instead of the input reg\n");
    printf("  -h                Show this help\n\n"); 
}

int get_int(char *s, int max)
{
    int val;
    char *endp;

    errno = 0;

    val = strtol(s, &endp, 0);

    if (errno) {
        perror("strtol");
        return -1;
    }

    if (endp == s) {
        printf("No digits found in %s\n", s);
        return -1;
    }

    if (val < 0 || val > max)
        return -1;

    return val;
}

int main(int argc, char **argv)
{
    int opt, ch, val;
    int opt_r = 0;
    int opt_w = 0;
    int opt_e = 0;

    while ((opt = getopt(argc, argv, "rweh")) != -1) {
        switch (opt) {
        case 'r':
            if (opt_w) {
                printf("Options -r and -w are mutually exclusive\n");
                usage(argv[0]);
                exit(1);
            }

            opt_r = 1;
            break;

        case 'w':
            if (opt_r) {
                printf("Options -r and -w are mutually exclusive\n");
                usage(argv[0]);
                exit(1);
            }

            opt_w = 1;
            break;
      
        case 'e':
            opt_e = 1;
            break;
 
        case 'h':
        default:
            usage(argv[0]);
            exit(0);
        }
    }

    if (opt_e && !(opt_r || opt_w)) {
        printf("-e option only applicable with -r or -w\n");
        usage(argv[0]);
        exit(1);
    }

    if (opt_r) {
        if (optind + 1 != argc) {
            printf("Read requires a channel\n");
            usage(argv[0]);
            exit(1);
        } 

        ch = get_int(argv[optind], 3);

        if (ch < 0) {
            printf("Invalid channel: %s\n", argv[optind]);
            usage(argv[0]);
            exit(1);
        }
    }
    else if (opt_w) {
        if (optind + 2 != argc) {
            printf("Write requires a channel and a value\n");
            usage(argv[0]);
            exit(1);
        }

        ch = get_int(argv[optind], 3);

        if (ch < 0) {
            printf("Invalid channel: %s\n", argv[optind]);
            usage(argv[0]);
            exit(1);
        } 

        val = get_int(argv[optind+1], 4095);
            
        if (val < 0) {
            printf("Invalid write value: %s\n", argv[optind+1]);
            usage(argv[0]);
            exit(1);
        }
    }

    if (opt_r) {
        val = qdac_read(ch, opt_e);

        if (val < 0)
            printf("qdac_read failed\n");
        else
            printf("%d\n", val);
    }
    else if (opt_w) {
        if (qdac_write(ch, val, opt_e) < 0)
            printf("qdac_write failed\n");
    }
    else {
        qdac_dump();
    }

    return 0;
}

