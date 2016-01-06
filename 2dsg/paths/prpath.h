#ifndef PATHPARSER_H
#define PATHPARSER_H

#include <stddef.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H

struct PrPath
{
    int numCommands;
    unsigned char *commands;
    int numCoords;
    float *coords;
};

#ifdef __cplusplus
extern "C" {
#endif

struct PrPath *prParseFtGlyph(FT_Outline *outline);
struct PrPath *prParseSvgPath(const char *pathString);
void prFreePath(struct PrPath *svgPath);

#ifdef __cplusplus
}
#endif

#endif
