/*
 * seq-tools
 * Copyright (c) 2019 Kirill Kryukov
 * See README.md and LICENSE files of this repository
 *
 * Usage: seq-soft-mask-bin-extract --mask MASK <IN >OUT
 */

#define TOOL_NAME "seq-soft-mask-bin-extract"
#include "common.c"


static char *mask_path = NULL;
static FILE *MASK = NULL;

static bool masked = false;
static unsigned long long length = 0ull;


static void done(void)
{
    free_in_buffer();
    if (MASK != NULL) { fclose_or_die(MASK); MASK = NULL; }
    fclose_or_die(stdout);
}


static void process(void)
{
    do
    {
        in_end = fread(in_buffer, 1, in_buffer_size, stdin);

        for (size_t i = 0; i < in_end; i++)
        {
            if ((in_buffer[i] >= 96) != masked)
            {
                fwrite_or_die(&length, sizeof(length), 1, MASK);
                masked = !masked;
                length = 0ull;
            }

            in_buffer[i] = (unsigned char)(in_buffer[i] & 0xDF);
            length++;
        }

        fwrite_or_die(in_buffer, 1, in_end, stdout);
    }
    while (in_end > 0);

    if (length > 0) { fwrite_or_die(&length, sizeof(length), 1, MASK); }
}


int main(int argc, char **argv)
{
    atexit(done);

    for (int i = 1; i < argc; i++)
    {
        if (i < argc - 1)
        {
            if (!strcmp(argv[i], "--mask")) { i++; mask_path = argv[i]; continue; }
        }
        die("Unknown or incomplete argument \"%s\"\n", argv[i]);
    }
    if (mask_path == NULL) { die("Mask file is not specified\n"); }

    change_io_to_binary_mode();
    allocate_in_buffer();

    MASK = fopen(mask_path, "wb");
    if (MASK == NULL) { die("Can't create mask file\n"); }

    process();

    return 0;
}
