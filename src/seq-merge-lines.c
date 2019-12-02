/*
 * seq-tools
 * Copyright (c) 2019 Kirill Kryukov
 * See README.md and LICENSE files of this repository
 *
 * Usage: seq-merge-lines <in.seq >out.seq
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
        in_begin = 0;
        in_end = fread(in_buffer, 1, in_buffer_size, stdin);
        if (in_end == 0) { break; }

        for (size_t i = 0; i < in_end; i++)
        {
            if (in_buffer[i] == 10)
            {
                fwrite_or_die(in_buffer + in_begin, 1, i - in_begin, stdout);
                in_begin = i + 1;
            }
        }

        fwrite_or_die(in_buffer + in_begin, 1, in_end - in_begin, stdout);
    }
}


int main(void)
{
    tool_name = "seq-merge-lines";
    atexit(done);
    change_io_to_binary_mode();
    allocate_in_buffer();

    process();

    return 0;
}
