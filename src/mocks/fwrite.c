
#include <stdio.h>

size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnonnull-compare"
    if (ptr != NULL && size == 1 && count == 1 && *(const char*)ptr == 'A')
    {
        return 0;
    }
#pragma GCC diagnostic pop

    for (size_t a = 0; a < size*count; a++)
    {
        fputc(((const char*)ptr)[a], stream);
    }

    return count;
}
