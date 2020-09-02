/*
 * seq-tools
 * Copyright (c) 2019-2020 Kirill Kryukov
 * See README.md and LICENSE files of this repository
 */

#define SOFTWARE_NAME "seq-tools"
#define VERSION "0.1.2-1"
#define DATE "2020-09-02"
#define COPYRIGHT_YEARS "2019-2020"


#include <assert.h>
#include <stddef.h>
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

#define MAX_N_FILES_TO_CLOSE 2u
static unsigned n_files_to_close = 0;
static FILE* files_to_close[MAX_N_FILES_TO_CLOSE] = { NULL, NULL };


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


static void register_file_to_close(FILE* f)
{
    assert(n_files_to_close < MAX_N_FILES_TO_CLOSE);
    files_to_close[n_files_to_close] = f;
    n_files_to_close++;
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
    void *buf = malloc(size);
    if (buf == NULL)
    {
        out_of_memory(size);
    }
    return buf;
}


static FILE* fopen_to_read_or_die(const char *path, const char *name)
{
    FILE *F = fopen(path, "rb");
    if (F == NULL)
    {
        die("Can't open %s", name);
    }
    register_file_to_close(F);
    return F;
}


static FILE* fopen_to_write_or_die(const char *path, const char *name)
{
    FILE *F = fopen(path, "wb");
    if (F == NULL)
    {
        die("Can't create %s", name);
    }
    register_file_to_close(F);
    return F;
}


static void allocate_in_buffer(void)
{
    in_buffer = (unsigned char *) malloc_or_die(in_buffer_size);
}


__attribute__((always_inline))
static inline void refill_in_buffer(void)
{
    in_begin = 0;
    in_end = fread(in_buffer, 1, in_buffer_size, stdin);
}


static void free_in_buffer(void)
{
    if (in_buffer)
    {
        free(in_buffer);
        in_buffer = NULL;
    }
}


__attribute__((always_inline))
static inline int in_get_char(void)
{
    if (in_begin >= in_end)
    {
        refill_in_buffer();
        if (in_end == 0) { return -1; }
    }
    return in_buffer[in_begin++];
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
}


static void unknown_tool(void)
{
    die("Unknown tool: \"%s\"", tool_name);
}


__attribute__ ((cold))
__attribute__ ((noreturn))
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
        void *t = malloc_or_die(12345);
        free(t);
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
        fputc_or_die(0, stdout);
    }
    else if (strcmp(task, "--fwrite") == 0)
    {
        fwrite_or_die("A", 1, 1, stdout);
    }
    else if (strcmp(task, "--fclose") == 0)
    {
        fclose_or_die(stdout);
    }
    else
    {
        die("Unknown task: \"%s\"", task);
    }

    exit(0);
}


static void tool_seq_t2u(void)
{
    for (;;)
    {
        in_end = fread(in_buffer, 1, in_buffer_size, stdin);
        if (in_end == 0)
        {
            break;
        }

        for (size_t i = 0; i < in_end; i++)
        {
            in_buffer[i] = (unsigned char)( in_buffer[i] ^ ((in_buffer[i] & 0xDFu) == 'T') );
        }

        fwrite_or_die(in_buffer, 1, in_end, stdout);
    }
}


static void tool_seq_u2t(void)
{
    for (;;)
    {
        in_end = fread(in_buffer, 1, in_buffer_size, stdin);
        if (in_end == 0)
        {
            break;
        }

        for (size_t i = 0; i < in_end; i++)
        {
            in_buffer[i] = (unsigned char)( in_buffer[i] ^ ((in_buffer[i] & 0xDFu) == 'U') );
        }

        fwrite_or_die(in_buffer, 1, in_end, stdout);
    }
}


static void tool_seq_change_case_to_upper(void)
{
    for (;;)
    {
        in_end = fread(in_buffer, 1, in_buffer_size, stdin);
        if (in_end == 0)
        {
            break;
        }

        for (size_t i = 0; i < in_end; i++)
        {
            in_buffer[i] = (unsigned char)(in_buffer[i] & 0xDFu);
        }

        fwrite_or_die(in_buffer, 1, in_end, stdout);
    }
}


static void tool_seq_merge_lines(void)
{
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
        die("Unknown or incomplete argument \"%s\"", args[i]);
    }
    if (!line_length_is_specified)
    {
        die("Line length is not specified");
    }
    if (line_length == 0ull)
    {
        die("Line length is 0");
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

    if ( line_rem != 0 &&
         line_rem != line_length )
    {
        fputc_or_die(10, stdout);
    }
}


static void tool_seq_soft_mask_add(int n_args, char **args)
{
    char *mask_path = NULL;
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
        die("Unknown or incomplete argument \"%s\"", args[i]);
    }
    if (mask_path == NULL)
    {
        die("Mask file is not specified");
    }

    FILE *MASK = fopen_to_read_or_die(mask_path, "mask file");

    for (;;)
    {
        unsigned long long length;

        // Processing unmasked sequence.
        if (fread(&length, sizeof(length), 1, MASK) != 1)
        {
            if ( in_end > in_begin ||
                 fgetc(stdin) != EOF )
            {
                die("Mask is shorter than input sequence");
            }
            return;
        }
    
        while (length > 0)
        {
            if (in_begin >= in_end)
            {
                refill_in_buffer();
                if (in_end == 0)
                {
                    die("Input sequence is shorter than mask");
                }
            }
    
            unsigned long long len1 = in_end - in_begin;
            if (len1 > length)
            {
                len1 = length;
            }
    
            fwrite_or_die(in_buffer + in_begin, 1, len1, stdout);
    
            length -= len1;
            in_begin += len1;
        }

        // Processing masked sequence.
        if (fread(&length, sizeof(length), 1, MASK) != 1)
        {
            if ( in_end > in_begin ||
                 fgetc(stdin) != EOF )
            {
                die("Mask is shorter than input sequence");
            }
            return;
        }

        // Suppressing Coverity Scan false positive.
        // While we don't verify the value of length, we still do want to use it as loop boundary.
        // coverity[tainted_data]
        while (length > 0)
        {
            if (in_begin >= in_end)
            {
                refill_in_buffer();
                if (in_end == 0)
                {
                    die("Input sequence is shorter than mask");
                }
            }
    
            unsigned long long len1 = in_end - in_begin;
            if (len1 > length)
            {
                len1 = length;
            }
    
            for (size_t i = in_begin; i < in_begin + len1; i++)
            {
                in_buffer[i] = (unsigned char)(in_buffer[i] | 0x20u);
            }
    
            fwrite(in_buffer + in_begin, 1, len1, stdout);
    
            length -= len1;
            in_begin += len1;
        }
    }
}


static void tool_seq_hard_mask_add(int n_args, char **args)
{
    char *mask_path = NULL;
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
        die("Unknown or incomplete argument \"%s\"", args[i]);
    }
    if (mask_path == NULL)
    {
        die("Mask file is not specified");
    }

    FILE *MASK = fopen_to_read_or_die(mask_path, "mask file");

    unsigned char *n_buffer = (unsigned char *) malloc_or_die(in_buffer_size);
    memset(n_buffer, 'N', in_buffer_size);

    for (;;)
    {
        unsigned long long length;

        // Processing unmasked sequence.
        if (fread(&length, sizeof(length), 1, MASK) != 1)
        {
            if ( in_end > in_begin ||
                 fgetc(stdin) != EOF )
            {
                die("Mask is shorter than input sequence");
            }
            break;
        }

        while (length > 0)
        {
            if (in_begin >= in_end)
            {
                refill_in_buffer();
                if (in_end == 0)
                {
                    die("Input sequence is shorter than mask");
                }
            }
    
            unsigned long long len1 = in_end - in_begin;
            if (len1 > length)
            {
                len1 = length;
            }
    
            fwrite_or_die(in_buffer + in_begin, 1, len1, stdout);
    
            length -= len1;
            in_begin += len1;
        }

        // Processing masked sequence.
        if (fread(&length, sizeof(length), 1, MASK) != 1)
        {
            if ( in_end > in_begin ||
                 fgetc(stdin) != EOF )
            {
                die("Mask is shorter than input sequence");
            }
            break;
        }

        // Suppressing Coverity Scan false positive.
        // While we don't verify the value of length, we still do want to use it as loop boundary.
        // coverity[tainted_data]
        while (length > in_buffer_size)
        {
            fwrite(n_buffer, 1, in_buffer_size, stdout);
            length -= in_buffer_size;
        }

        fwrite(n_buffer, 1, length, stdout);
    }

    free(n_buffer);
}


static void tool_seq_soft_mask_extract(int n_args, char **args)
{
    char *mask_path = NULL;
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
        die("Unknown or incomplete argument \"%s\"", args[i]);
    }
    if (mask_path == NULL)
    {
        die("Mask file is not specified");
    }

    FILE *MASK = fopen_to_write_or_die(mask_path, "mask file");

    bool masked = false;
    unsigned long long length = 0ull;

    do
    {
        in_end = fread(in_buffer, 1, in_buffer_size, stdin);
        unsigned long long mask_begin = 0ull;

        for (size_t i = 0; i < in_end; i++)
        {
            if ((in_buffer[i] >= 96) != masked)
            {
                length += i - mask_begin;
                fwrite_or_die(&length, sizeof(length), 1, MASK);
                masked = !masked;
                length = 0ull;
                mask_begin = i;
            }

            in_buffer[i] = (unsigned char)(in_buffer[i] & 0xDFu);
        }

        length += in_end - mask_begin;
        fwrite_or_die(in_buffer, 1, in_end, stdout);
    }
    while (in_end > 0);

    if (length > 0)
    {
        fwrite_or_die(&length, sizeof(length), 1, MASK);
    }
}


static void tool_seq_hard_mask_extract(int n_args, char **args)
{
    char *mask_path = NULL;
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
        die("Unknown or incomplete argument \"%s\"", args[i]);
    }
    if (mask_path == NULL)
    {
        die("Mask file is not specified");
    }

    FILE *MASK = fopen_to_write_or_die(mask_path, "mask file");

    bool masked = false;
    unsigned long long length = 0ull;

    do
    {
        in_begin = 0;
        in_end = fread(in_buffer, 1, in_buffer_size, stdin);

        for (size_t i = 0; i < in_end; i++)
        {
            if ((in_buffer[i] == 'N') != masked)
            {
                length += i - in_begin;
                fwrite_or_die(&length, sizeof(length), 1, MASK);
                if (!masked)
                {
                    fwrite_or_die(in_buffer + in_begin, 1, i - in_begin, stdout);
                }
                masked = !masked;
                length = 0ull;
                in_begin = i;
            }
        }

        length += in_end - in_begin;
        if (!masked)
        {
            fwrite_or_die(in_buffer + in_begin, 1, in_end - in_begin, stdout);
        }
    }
    while (in_end > 0);

    if (length > 0)
    {
        fwrite_or_die(&length, sizeof(length), 1, MASK);
    }
}


static void tool_seq_iupac_add(int n_args, char **args)
{
    char *iupac_path = NULL;
    for (int i = 0; i < n_args; i++)
    {
        if (i < n_args - 1)
        {
            if (strcmp(args[i], "--iupac") == 0)
            {
                i++;
                iupac_path = args[i];
                continue;
            }
        }
        die("Unknown or incomplete argument \"%s\"", args[i]);
    }
    if (iupac_path == NULL)
    {
        die("IUPAC file is not specified");
    }

    FILE *IUPAC = fopen_to_read_or_die(iupac_path, "IUPAC file");

    unsigned long long shift;
    unsigned char c;
    while ( fread(&shift, sizeof(shift), 1, IUPAC) == 1 &&
            fread(&c, sizeof(c), 1, IUPAC) == 1)
    {
        // Suppressing Coverity Scan false positive.
        // While we don't verify the value of 'shift', we still do want to use it as loop boundary.
        // coverity[tainted_data]
        while (shift > 0)
        {
            if (in_begin >= in_end)
            {
                in_begin = 0;
                in_end = fread(in_buffer, 1, in_buffer_size, stdin);
                if (in_end == 0)
                {
                    die("Input sequence is too short");
                }
            }

            unsigned long long len1 = in_end - in_begin;
            if (len1 > shift)
            {
                len1 = shift;
            }

            fwrite_or_die(in_buffer + in_begin, 1, len1, stdout);

            shift -= len1;
            in_begin += len1;
        }

        fputc_or_die(c, stdout);
    }

    do
    {
        fwrite_or_die(in_buffer + in_begin, 1, in_end - in_begin, stdout);
        in_begin = 0;
        in_end = fread(in_buffer, 1, in_buffer_size, stdin);
    }
    while (in_end > 0);
}


static void tool_seq_iupac_extract(int n_args, char **args)
{
    char *iupac_path = NULL;
    for (int i = 0; i < n_args; i++)
    {
        if (i < n_args - 1)
        {
            if (strcmp(args[i], "--iupac") == 0)
            {
                i++;
                iupac_path = args[i];
                continue;
            }
        }
        die("Unknown or incomplete argument \"%s\"", args[i]);
    }
    if (iupac_path == NULL)
    {
        die("IUPAC file is not specified");
    }

    FILE *IUPAC = fopen_to_write_or_die(iupac_path, "IUPAC file");

    bool is_iupac_arr[256];
    memset(is_iupac_arr, 0, sizeof(is_iupac_arr));
    is_iupac_arr['R'] = true;
    is_iupac_arr['Y'] = true;
    is_iupac_arr['S'] = true;
    is_iupac_arr['W'] = true;
    is_iupac_arr['K'] = true;
    is_iupac_arr['M'] = true;
    is_iupac_arr['B'] = true;
    is_iupac_arr['D'] = true;
    is_iupac_arr['H'] = true;
    is_iupac_arr['V'] = true;

    unsigned long long shift = 0ull;

    do
    {
        in_begin = 0;
        in_end = fread(in_buffer, 1, in_buffer_size, stdin);

        for (size_t i = 0; i < in_end; i++)
        {
            if (is_iupac_arr[in_buffer[i]])
            {
                fwrite_or_die(in_buffer + in_begin, 1, i - in_begin, stdout);

                shift += i - in_begin;
                fwrite(&shift, sizeof(shift), 1, IUPAC);
                fputc(in_buffer[i], IUPAC);

                shift = 0ull;
                in_begin = i + 1;
            }
        }

        shift += in_end - in_begin;
        fwrite(in_buffer + in_begin, 1, in_end - in_begin, stdout);
    }
    while (in_end > 0);
}


__attribute__((always_inline))
static inline int skip_end_of_line(void)
{
    for (;;)
    {
        if (in_begin >= in_end)
        {
            refill_in_buffer();
            if (in_end == 0)
            {
                return -1;
            }
        }

        while (in_begin < in_end)
        {
            if (in_buffer[in_begin] != 10 && in_buffer[in_begin] != 13)
            {
                return in_buffer[in_begin];
            }
            in_begin++;
        }
    }
}


__attribute__((always_inline))
static inline int fasta_print_name(void)
{
    int c = -1;
    do
    {
        if (in_begin >= in_end)
        {
            refill_in_buffer();
            if (in_end == 0)
            {
                break;
            }
        }

        size_t i;
        for (i = in_begin; i < in_end; i++)
        {
            if (in_buffer[i] == 10 || in_buffer[i] == 13)
            {
                c = 10;
                break;
            }
        }

        if (i == in_end)
        {
            i--;
        }
        fwrite_or_die(in_buffer + in_begin, 1, i - in_begin + 1, stdout);
        in_begin = i + 1;
    }
    while (c < 0);
    return c;
}


__attribute__((always_inline))
static inline int fasta_print_seq_one_line(void)
{
    for (;;)
    {
        if (in_begin >= in_end)
        {
            refill_in_buffer();
            if (in_end == 0)
            {
                fputc_or_die('\n', stdout);
                return -1;
            }
        }

        size_t i;
        for (i = in_begin; i < in_end; i++)
        {
            if (in_buffer[i] == 10 || in_buffer[i] == 13)
            {
                fwrite_or_die(in_buffer + in_begin, 1, i - in_begin, stdout);

                do
                {
                    i++;
                    if (i >= in_end)
                    {
                        refill_in_buffer();
                        if (in_end == 0)
                        {
                            fputc_or_die('\n', stdout);
                            return -1;
                        }
                        i = in_begin;
                    }
                }
                while (in_buffer[i] == 10 || in_buffer[i] == 13);

                in_begin = i;

                if (in_buffer[i] == '>')
                {
                    fputc_or_die('\n', stdout);
                    return '>';
                }
            }
        }

        if (i > in_begin)
        {
            fwrite_or_die(in_buffer + in_begin, 1, i - in_begin, stdout);
        }
        in_begin = i;
    }
}


static void tool_fasta_unwrap_lines(void)
{
    int c = skip_end_of_line();
    if (c != '>')
    {
        die("Input is not in FASTA format");
        exit(1);
    }

    while (c >= 0)
    {
        c = fasta_print_name();
        if (c < 0)
        {
            break;
        }

        c = skip_end_of_line();
        if (c < 0)
        {
            break;
        }
        else if (c == '>')
        {
            continue;
        }

        c = fasta_print_seq_one_line();
    }
}


int main(int argc, char **argv)
{
    if (argc < 2)
    {
        die("Tool is not specified");
    }

    tool_name = argv[1];
    int n_args = argc - 2;
    char **args = (argc > 2) ? (argv + 2) : NULL;

    if (strcmp(tool_name, "test-dummy") == 0)
    {
        tool_test_dummy(n_args, args);
    }

    atexit(done);
    change_io_to_binary_mode();
    register_file_to_close(stdout);
    allocate_in_buffer();

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
        else if (strcmp(tool_suffix, "soft-mask-add") == 0)
        {
            tool_seq_soft_mask_add(n_args, args);
        }
        else if (strcmp(tool_suffix, "soft-mask-extract") == 0)
        {
            tool_seq_soft_mask_extract(n_args, args);
        }
        else if (strcmp(tool_suffix, "hard-mask-add") == 0)
        {
            tool_seq_hard_mask_add(n_args, args);
        }
        else if (strcmp(tool_suffix, "hard-mask-extract") == 0)
        {
            tool_seq_hard_mask_extract(n_args, args);
        }
        else if (strcmp(tool_suffix, "iupac-add") == 0)
        {
            tool_seq_iupac_add(n_args, args);
        }
        else if (strcmp(tool_suffix, "iupac-extract") == 0)
        {
            tool_seq_iupac_extract(n_args, args);
        }
        else
        {
            unknown_tool();
        }
    }
    else if (strncmp(tool_name, "fasta-", 6) == 0)
    {
        const char *tool_suffix = tool_name + 6;
        if (strcmp(tool_suffix, "unwrap-lines") == 0)
        {
            tool_fasta_unwrap_lines();
        }
        else
        {
            unknown_tool();
        }
    }
    else
    {
        unknown_tool();
    }

    return 0;
}
