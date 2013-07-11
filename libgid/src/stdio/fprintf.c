#include <stdio.h>
#include <stdarg.h>
#include "local.h"

int
fprintf(FILE *fp, const char *fmt, ...)
{
    int ret;
    va_list ap;

    va_start(ap, fmt);
    ret = vfprintf(fp, fmt, ap);
    va_end(ap);
    return (ret);
}
