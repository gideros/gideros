#ifndef SVGPATHPARSER_H
#define SVGPATHPARSER_H

#include <stddef.h>

struct PrSvgPath
{
    int numCommands;
    unsigned char *commands;
    int numCoords;
    float *coords;
};

#ifdef __cplusplus
extern "C" {
#endif

struct PrSvgPath *prParseSvgPath(size_t length, const char *pathString);
void prFreeSvgPath(struct PrSvgPath *svgPath);

#ifdef __cplusplus
}
#endif

#endif