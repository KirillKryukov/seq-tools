
#include <stddef.h>
#include <stdlib.h>

void* malloc(size_t size)
{
    return (size == 12345) ? 0 : calloc(1, size);
}
