#include <stdio.h>
#include <stdarg.h>
#include "local.h"

int
vfprintf(FILE *fp, const char *fmt, va_list ap)
{
    int len;
    char* buf;
    int res;
    va_list ap2;

    va_copy(ap2, ap);
    len = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);
    buf = malloc(len + 1);
    vsprintf(buf, fmt, ap);
    res = fwrite(buf, 1, len, fp);
    free(buf);

    return res;
}
