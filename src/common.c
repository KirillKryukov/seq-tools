/*
 * seq-tools
 * Copyright (c) 2019 Kirill Kryukov
 * See README.md and LICENSE files of this repository
 */

#include <stdlib.h>
#include <stdio.h>


#define in_buffer_size 16384
static unsigned char *in_buffer = NULL;
static size_t in_begin = 0;
static size_t in_end = 0;


static void allocate_in_buffer(void)
{
    in_buffer = (unsigned char *) malloc(in_buffer_size);
    if (in_buffer == NULL) { fputs("Can't allocate memory\n", stderr); exit(1); }
}


static void free_in_buffer(void)
{
    if (in_buffer) { free(in_buffer); in_buffer = NULL; }
}


static void change_io_to_binary_mode(void)
{
    if (!freopen(NULL, "rb", stdin)) { fputs("Can't read input in binary mode\n", stderr); exit(1); }
    if (!freopen(NULL, "wb", stdout)) { fputs("Can't set output stream to binary mode\n", stderr); exit(1); }
}
