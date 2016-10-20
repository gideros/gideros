#include "kvec.h"
#include <ctype.h>
#include <setjmp.h>
#include "prpath.h"

struct ParseData
{
    const char *str;
    jmp_buf buf;
};

static void wsp(struct ParseData *d)
{
    for (;;)
    {
        if (isspace(*d->str))
            d->str++;
        else
            break;
    }
}

static void comma(struct ParseData *d)
{
    wsp(d);

    if (*d->str == ',')
        d->str++;
    else
        return;

    wsp(d);
}

static float number(struct ParseData *d)
{
    if (isspace(*d->str))
        longjmp(d->buf, 1);
    char *end = NULL;
    float result = (float)strtod(d->str, &end);
    if (d->str == end)
        longjmp(d->buf, 1);
    d->str = end;
    return result;
}

static int hasNumber(struct ParseData *d)
{
    if (isspace(*d->str))
        return 0;
    char *end = NULL;
    strtod(d->str, &end);
    return d->str != end;
}

static int hasComma(struct ParseData *d)
{
    const char *save = d->str;
    wsp(d);
    int result = (*d->str == ',');
    d->str = save;
    return result;
}

typedef kvec_t(float) kvec_float_t;

static void numbers(struct ParseData *d, int alignment, int multiple, kvec_float_t *v)
{
    int i;

    wsp(d);

    for (;;)
    {
        for (i = 0; i < alignment; ++i)
        {
            if (i == 0)
            {
                kv_push_back(*v, number(d));
            }
            else
            {
                comma(d);
                kv_push_back(*v, number(d));
            }
        }

        if (!multiple)
            break;

        if (hasComma(d))
        {
            comma(d);
            continue;
        }

        wsp(d);

        if (!hasNumber(d))
            break;
    }
}

typedef kvec_t(unsigned char) kvec_uchar_t;

static void parseHelper(struct ParseData *d, kvec_uchar_t *commands, kvec_float_t *coords, int alignment, int multiple)
{
    int i;
    size_t j;
    char command;
    kvec_float_t n;

    command = *d->str;
    d->str++;

    kv_init(n);
    numbers(d, alignment, multiple, &n);

    if (multiple)
    {
        for (j = 0; j < kv_size(n); j += alignment)
        {
            kv_push_back(*commands, command);
            for (i = 0; i < alignment; ++i)
                kv_push_back(*coords, kv_a(n, j + i));
        }
    }
    else
    {
        kv_push_back(*commands, command);
        for (i = 0; i < alignment; ++i)
            kv_push_back(*coords, kv_a(n, i));
    }

    kv_free(n);
}

static void parse(struct ParseData *d, kvec_uchar_t *commands, kvec_float_t *coords)
{
    for (;;)
    {
        wsp(d);

        if (*d->str == 0)
            break;

        switch (*d->str)
        {
        case 'M':
        case 'm':
            parseHelper(d, commands, coords, 2, 0);
            break;
        case 'L':
        case 'l':
            parseHelper(d, commands, coords, 2, 1);
            break;
        case 'Q':
        case 'q':
            parseHelper(d, commands, coords, 4, 1);
            break;
        case 'C':
        case 'c':
            parseHelper(d, commands, coords, 6, 1);
            break;
        case 'H':
        case 'h':
            parseHelper(d, commands, coords, 1, 1);
            break;
        case 'V':
        case 'v':
            parseHelper(d, commands, coords, 1, 1);
            break;
        case 'A':
        case 'a':
            parseHelper(d, commands, coords, 7, 1);
            break;
        case 'T':
        case 't':
            parseHelper(d, commands, coords, 2, 1);
            break;
        case 'S':
        case 's':
            parseHelper(d, commands, coords, 4, 1);
            break;
        case 'Z':
        case 'z':
            d->str++;
            kv_push_back(*commands, 'Z');
            break;
        default:
            longjmp(d->buf, 1);
            break;
        }
    }
}

struct PrPath *prParseSvgPath(const char *pathString)
{
    struct ParseData d;
    
    kvec_uchar_t commands;
    kvec_float_t coords;

    kv_init(commands);
    kv_init(coords);

    d.str = pathString;

    if (!setjmp(d.buf))
        parse(&d, &commands, &coords);
    else
        return NULL;

    struct PrPath *svgPath = (struct PrPath*)malloc(sizeof(struct PrPath));

    if (kv_empty(commands))
    {
        svgPath->numCommands = 0;
        svgPath->commands = NULL;
    }
    else
    {
        svgPath->numCommands = kv_size(commands);
        svgPath->commands = (unsigned char*)malloc(svgPath->numCommands * sizeof(unsigned char));
        memcpy(svgPath->commands, kv_data(commands), svgPath->numCommands * sizeof(unsigned char));
    }

    if (kv_empty(coords))
    {
        svgPath->numCoords = 0;
        svgPath->coords = NULL;
    }
    else
    {
        svgPath->numCoords = kv_size(coords);
        svgPath->coords = (float*)malloc(svgPath->numCoords * sizeof(float));
        memcpy(svgPath->coords, kv_data(coords), svgPath->numCoords * sizeof(float));
    }

    kv_free(commands);
    kv_free(coords);

    return svgPath;
}
