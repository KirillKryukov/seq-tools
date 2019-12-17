/*
 * seq-tools
 * Copyright (c) 2019 Kirill Kryukov
 * See README.md and LICENSE files of this repository
 */

#define SOFTWARE_NAME "seq-tools"
#define VERSION "0.1.0"
#define DATE "2019-12-17"
#define COPYRIGHT_YEARS "2019"


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __MINGW32__
#include <fcntl.h>
#endif

#if defined(__MINGW32__) || defined(__MINGW64__) || defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
#define PRINT_ULL "I64u"
#define PRINT_SIZE_T "Iu"
#else
#define PRINT_ULL "llu"
#define PRINT_SIZE_T "zu"
#endif


#define in_buffer_size 16384
static unsigned char *in_buffer = NULL;
static size_t in_begin = 0;
static size_t in_end = 0;

static const char* tool_name = NULL;

#define n_files_to_close 1
static FILE* files_to_close[n_files_to_close] = {NULL};


__attribute__ ((cold))
__attribute__ ((format (printf, 1, 2)))
__attribute__ ((noreturn))
static void die(const char *format, ...) 
{
    fputs(SOFTWARE_NAME, stderr);
    if (tool_name != NULL)
    {
        fputc(' ', stderr);
        fputs(tool_name, stderr);
    }
    fputs(" error: ", stderr);
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    fputc('\n', stderr);
    exit(1);
}


__attribute__ ((cold))
__attribute__ ((noreturn))
static void out_of_memory(const size_t size)
{
    die("Can't allocate %" PRINT_SIZE_T " bytes", size);
}


__attribute__ ((malloc))
__attribute__ ((alloc_size (1)))
static void* malloc_or_die(const size_t size)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Walloc-size-larger-than="
    void *buf = malloc(size);
#pragma GCC diagnostic pop
    if (buf == NULL) { out_of_memory(size); }
    return buf;
}


static void allocate_in_buffer(void)
{
    in_buffer = (unsigned char *) malloc_or_die(in_buffer_size);
    (void)in_begin;
    (void)in_end;
}


static void free_in_buffer(void)
{
    if (in_buffer)
    {
        free(in_buffer);
        in_buffer = NULL;
    }
}


static void change_io_to_binary_mode(void)
{
#ifdef __MINGW32__
    if (_setmode(_fileno(stdout), O_BINARY) == -1)
#else
    if (!freopen(NULL, "wb", stdout))
#endif
    {
        die("Can't set output stream to binary mode");
    }

#ifdef __MINGW32__
    if (_setmode(_fileno(stdin), O_BINARY) == -1)
#else
    if (!freopen(NULL, "rb", stdin))
#endif
    {
        die("Can't read input in binary mode");
    }
}


__attribute__((always_inline))
static inline void fputc_or_die(int c, FILE *F)
{
    if (fputc(c, F) != c)
    {
        die("Write failure - disk full?");
    }
}


__attribute__((always_inline))
static inline void fwrite_or_die(const void *ptr, size_t element_size, size_t n_elements, FILE *F)
{
    size_t elements_written = fwrite(ptr, element_size, n_elements, F);
    if (elements_written != n_elements)
    {
        die("Write failure - disk full?");
    }
}


static void fclose_or_die(FILE *F)
{
    int error = fclose(F);
    if (error != 0)
    {
        die("Can't close output stream - disk full?");
    }
}


static void done(void)
{
    for (unsigned i = 0; i < n_files_to_close; i++)
    {
        if (files_to_close[i] != NULL)
        {
            fclose_or_die(files_to_close[i]);
            files_to_close[i] = NULL;
        }
    }

    free_in_buffer();
    fclose_or_die(stdout);
}


static void unknown_tool(void)
{
    die("Unknown tool: \"%s\"", tool_name);
}


static void tool_test_dummy(int n_args, char **args)
{
    if (n_args < 1)
    {
        die("Task is not specified");
    }
    if (n_args > 1)
    {
        die("Unknown argument: \"%s\"", args[1]);
    }

    const char *task = args[0];
    if (strcmp(task, "--out-of-memory") == 0)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Walloc-size-larger-than="
        malloc_or_die(SIZE_MAX);
#pragma GCC diagnostic pop
    }
    else if (strcmp(task, "--binary-stdin") == 0)
    {
        fclose(stdin);
        change_io_to_binary_mode();
    }
    else if (strcmp(task, "--binary-stdout") == 0)
    {
        fclose(stdout);
        change_io_to_binary_mode();
    }
    else if (strcmp(task, "--fputc") == 0)
    {
        fclose(stdout);
        fputc_or_die('A', stdout);
    }
    else if (strcmp(task, "--fwrite") == 0)
    {
        fclose(stdout);
        fwrite_or_die("A", 1, 1, stdout);
    }
    else if (strcmp(task, "--fclose") == 0)
    {
        fclose_or_die(stdout);
    }

    die("Unknown task: \"%s\"", task);
}


static void tool_seq_t2u(void)
{
    atexit(done);
    change_io_to_binary_mode();
    allocate_in_buffer();

    for (;;)
    {
        in_end = fread(in_buffer, 1, in_buffer_size, stdin);
        if (in_end == 0)
        {
            break;
        }

        for (size_t i = 0; i < in_end; i++)
        {
            in_buffer[i] = (unsigned char)( in_buffer[i] ^ ((in_buffer[i] & 0xDF) == 'T') );
        }

        fwrite_or_die(in_buffer, 1, in_end, stdout);
    }
}


static void tool_seq_u2t(void)
{
    atexit(done);
    change_io_to_binary_mode();
    allocate_in_buffer();

    for (;;)
    {
        in_end = fread(in_buffer, 1, in_buffer_size, stdin);
        if (in_end == 0)
        {
            break;
        }

        for (size_t i = 0; i < in_end; i++)
        {
            in_buffer[i] = (unsigned char)( in_buffer[i] ^ ((in_buffer[i] & 0xDF) == 'U') );
        }

        fwrite_or_die(in_buffer, 1, in_end, stdout);
    }
}


static void tool_seq_change_case_to_upper(void)
{
    atexit(done);
    change_io_to_binary_mode();
    allocate_in_buffer();

    for (;;)
    {
        in_end = fread(in_buffer, 1, in_buffer_size, stdin);
        if (in_end == 0)
        {
            break;
        }

        for (size_t i = 0; i < in_end; i++)
        {
            in_buffer[i] = (unsigned char)(in_buffer[i] & 0xDF);
        }

        fwrite_or_die(in_buffer, 1, in_end, stdout);
    }
}


static void tool_seq_merge_lines(void)
{
    atexit(done);
    change_io_to_binary_mode();
    allocate_in_buffer();

    for (;;)
    {
        in_begin = 0;
        in_end = fread(in_buffer, 1, in_buffer_size, stdin);
        if (in_end == 0)
        {
            break;
        }

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


static void tool_seq_split_to_lines(int n_args, char **args)
{
    atexit(done);
    change_io_to_binary_mode();
    allocate_in_buffer();

    bool line_length_is_specified = false;
    unsigned long long line_length = 0ull;

    for (int i = 0; i < n_args; i++)
    {
        if (i < n_args - 1)
        {
            if (strcmp(args[i], "--line-length") == 0)
            {
                i++;
                line_length = strtoull(args[i], NULL, 10);
                line_length_is_specified = true;
                continue;
            }
        }
        die("Unknown or incomplete argument \"%s\"\n", args[i]);
    }
    if (!line_length_is_specified)
    {
        die("Line length is not specified\n");
    }
    if (line_length == 0ull)
    {
        die("Line length is 0\n");
    }

    unsigned long long line_rem = 0ull;

    for (;;)
    {
        in_begin = 0;
        in_end = fread(in_buffer, 1, in_buffer_size, stdin);
        if (in_end == 0)
        {
            break;
        }

        if (line_rem > 0)
        {
            unsigned long long len1 = in_end - in_begin;
            if (len1 > line_rem)
            {
                len1 = line_rem;
            }
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

    if (line_rem != 0 && line_rem != line_length)
    {
        fputc_or_die(10, stdout);
    }
}


static void tool_seq_soft_mask_bin_extract(int n_args, char **args)
{
    atexit(done);
    change_io_to_binary_mode();
    allocate_in_buffer();

    char *mask_path = NULL;
    FILE *MASK = NULL;
    bool masked = false;
    unsigned long long length = 0ull;

    for (int i = 0; i < n_args; i++)
    {
        if (i < n_args - 1)
        {
            if (strcmp(args[i], "--mask") == 0)
            {
                i++;
                mask_path = args[i];
                continue;
            }
        }
        die("Unknown or incomplete argument \"%s\"\n", args[i]);
    }
    if (mask_path == NULL)
    {
        die("Mask file is not specified\n");
    }

    MASK = fopen(mask_path, "wb");
    if (MASK == NULL)
    {
        die("Can't create mask file\n");
    }
    files_to_close[0] = MASK;

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
    if (argc < 2)
    {
        die("Tool is not specified");
    }

    tool_name = argv[1];
    int n_args = argc - 2;
    char **args = argv + 2;

    if (strncmp(tool_name, "seq-", 4) == 0)
    {
        const char *tool_suffix = tool_name + 4;
        if (strcmp(tool_suffix, "t2u") == 0)
        {
            tool_seq_t2u();
        }
        else if (strcmp(tool_suffix, "u2t") == 0)
        {
            tool_seq_u2t();
        }
        else if (strcmp(tool_suffix, "change-case-to-upper") == 0)
        {
            tool_seq_change_case_to_upper();
        }
        else if (strcmp(tool_suffix, "merge-lines") == 0)
        {
            tool_seq_merge_lines();
        }
        else if (strcmp(tool_suffix, "split-to-lines") == 0)
        {
            tool_seq_split_to_lines(n_args, args);
        }
        else if (strcmp(tool_suffix, "soft-mask-bin-extract") == 0)
        {
            tool_seq_soft_mask_bin_extract(n_args, args);
        }
        else
        {
            unknown_tool();
        }
    }
    else if (strcmp(tool_name, "test-dummy") == 0)
    {
        tool_test_dummy(n_args, args);
    }
    else
    {
        unknown_tool();
    }

    return 0;
}