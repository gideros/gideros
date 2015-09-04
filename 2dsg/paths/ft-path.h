#ifndef FT_PATH_H
#define FT_PATH_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H

struct path
{
    int num_commands;
    unsigned char *commands;
    int num_coords;
    float *coords;
};

struct path *path_from_glyph(FT_Outline *outline);
void free_path(struct path *p);

#endif
