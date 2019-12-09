/*
 * seq-tools
 * Copyright (c) 2019 Kirill Kryukov
 * See README.md and LICENSE files of this repository
 *
 * Usage: seq-split-to-lines --line-length 100 <in.seq >out.seq
 */

#define TOOL_NAME "seq-split-to-lines"
#include "common.c"


static unsigned long long line_length = 0ull;


static void done(void)
{
    free_in_buffer();
    fclose_or_die(stdout);
}


static void process(void)
{
    unsigned long long line_rem = 0ull;

    for (;;)
    {
        in_begin = 0;
        in_end = fread(in_buffer, 1, in_buffer_size, stdin);
        if (in_end == 0) { break; }

        if (line_rem > 0)
        {
            unsigned long long len1 = in_end - in_begin;
            if (len1 > line_rem) { len1 = line_rem; }
            fwrite_or_die(in_buffer, 1, len1, stdout);
            fputc_or_die(10, stdout);
            in_begin += len1;
            line_rem -= len1;
        }

        for (size_t i = in_begin + line_length; i <= in_end; i += line_length)
        {
            fwrite_or_die(in_buffer + in_begin, 1, line_length, stdout);
            fputc_or_die(10, stdout);
            in_begin = i;
        }

        if (in_begin < in_end)
        {
            unsigned long long len1 = in_end - in_begin;
            fwrite_or_die(in_buffer + in_begin, 1, len1, stdout);
            line_rem = line_length - len1;
        }
    }

    if (line_rem != 0 && line_rem != line_length) { fputc_or_die(10, stdout); }
}


int main(int argc, char **argv)
{
    atexit(done);

    for (int i = 1; i < argc; i++)
    {
        if (i < argc - 1)
        {
            if (!strcmp(argv[i], "--line-length")) { i++; line_length = strtoull(argv[i], NULL, 10); continue; }
        }
        die("Unknown or incomplete argument \"%s\"\n", argv[i]);
    }
    if (line_length == 0ull) { die("Line length is not specified\n"); }

    change_io_to_binary_mode();
    allocate_in_buffer();

    process();

    return 0;
}
