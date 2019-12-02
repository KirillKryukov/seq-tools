/*
 * seq-tools
 * Copyright (c) 2019 Kirill Kryukov
 * See README.md and LICENSE files of this repository
 *
 * Usage: seq-t2u <in.seq >out.seq
 */

#include "common.c"


static void done(void)
{
    free_in_buffer();
    fclose_or_die(stdout);
}


static void process(void)
{
    for (;;)
    {
        in_end = fread(in_buffer, 1, in_buffer_size, stdin);
        if (in_end == 0) { break; }

        for (size_t i = 0; i < in_end; i++)
        {
            in_buffer[i] = (unsigned char)( in_buffer[i] ^ ((in_buffer[i] & 0xDF) == 'T') );
        }

        fwrite_or_die(in_buffer, 1, in_end, stdout);
    }
}


int main(void)
{
    tool_name = "seq-t2u";
    atexit(done);
    change_io_to_binary_mode();
    allocate_in_buffer();

    process();

    return 0;
}
