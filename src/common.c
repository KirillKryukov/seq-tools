/*
 * seq-tools
 * Copyright (c) 2019 Kirill Kryukov
 * See README.md and LICENSE files of this repository
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

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


__attribute__ ((cold))
__attribute__ ((format (printf, 1, 2)))
__attribute__ ((noreturn))
static void die(const char *format, ...) 
{
    fputs(TOOL_NAME " error: ", stderr);
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    exit(1);
}


__attribute__ ((cold))
__attribute__ ((noreturn))
static void out_of_memory(const size_t size)
{
    die("Can't allocate %" PRINT_SIZE_T " bytes\n", size);
}


static void* malloc_or_die(const size_t size)
{
    void *buf = malloc(size);
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
    if (in_buffer) { free(in_buffer); in_buffer = NULL; }
}


static void change_io_to_binary_mode(void)
{
#ifdef __MINGW32__
    if (_setmode(_fileno(stdout), O_BINARY) == -1)
#else
    if (!freopen(NULL, "wb", stdout))
#endif
        { die("Can't set output stream to binary mode\n"); }

#ifdef __MINGW32__
    if (_setmode(_fileno(stdin), O_BINARY) == -1)
#else
    if (!freopen(NULL, "rb", stdin))
#endif
        { die("can't read input in binary mode\n"); }
}


__attribute__((always_inline))
static inline void fputc_or_die(int c, FILE *F)
{
    if (fputc(c, F) != c) { die("Write failure - disk full?\n"); }
}


__attribute__((always_inline))
static inline void fwrite_or_die(const void *ptr, size_t element_size, size_t n_elements, FILE *F)
{
    size_t elements_written = fwrite(ptr, element_size, n_elements, F);
    if (elements_written != n_elements) { die("Write failure - disk full?\n"); }
}


static void fclose_or_die(FILE *F)
{
    int error = fclose(F);
    if (error != 0) { die("Can't close output stream - disk full?\n"); }
}
