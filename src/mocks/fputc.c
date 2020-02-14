
#include <stdio.h>

int fputc(int c, FILE *stream)
{
    if (c == 0) { return 1; }
    char s[2] = {(char)c, 0};
    return fputs(s, stream);
}
