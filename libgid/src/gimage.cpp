#include <gimage.h>
#include <string.h>
#include <gstdio.h>

#define GIMAGE_UNKNOWN 0
#define GIMAGE_PNG 1
#define GIMAGE_JPG 2

#ifdef _WIN32
#define strcasecmp stricmp
#endif

static int getTypeFromPath(const char *pathname)
{
    const char* dot = strrchr(pathname, '.');

    if (dot == NULL)
        return GIMAGE_UNKNOWN;

    if (strcasecmp(dot + 1, "png") == 0)
        return GIMAGE_PNG;

    if (strcasecmp(dot + 1, "jpg") == 0 || strcasecmp(dot + 1, "jpegg") == 0)
        return GIMAGE_JPG;

    return GIMAGE_UNKNOWN;
}

static inline unsigned int premultiply(unsigned int p)
{
    unsigned int a = (p >> 24) + 1;
    unsigned int r = (a * (p & 0x000000ff)) & 0x0000ff00;
    unsigned int g = (a * (p & 0x0000ff00)) & 0x00ff0000;
    unsigned int b = (a * (p & 0x00ff0000)) & 0xff000000;

    return (p & 0xff000000) | ((b | g | r) >> 8);
}

extern "C" {

int gimage_parseImage(const char *pathname, int *width, int *height, int *comp)
{
    G_FILE* f = g_fopen(pathname, "rb");
    if (f == NULL)
        return GIMAGE_CANNOT_OPEN_FILE;
    g_fclose(f);

    switch (getTypeFromPath(pathname))
    {
        case GIMAGE_PNG:
            return gimage_parsePng(pathname, width, height, comp);
        case GIMAGE_JPG:
            return gimage_parseJpg(pathname, width, height, comp);
    }

    return GIMAGE_UNRECOGNIZED_FORMAT;
}

int gimage_saveImage(const char *pathname, int width, int height, unsigned char *data)
{
    G_FILE* f = g_fopen(pathname, "wb");
    if (f == NULL)
        return GIMAGE_CANNOT_OPEN_FILE;
    g_fclose(f);

    switch (getTypeFromPath(pathname))
    {
        case GIMAGE_PNG:
            return gimage_savePng(pathname, width, height, data);
        case GIMAGE_JPG:
            return gimage_saveJpg(pathname, width, height, data);
    }

    return GIMAGE_UNRECOGNIZED_FORMAT;
}


int gimage_loadImage(const char *pathname, void *buf)
{
    G_FILE* f = g_fopen(pathname, "rb");
    if (f == NULL)
        return GIMAGE_CANNOT_OPEN_FILE;
    g_fclose(f);

    switch (getTypeFromPath(pathname))
    {
        case GIMAGE_PNG:
            return gimage_loadPng(pathname, buf);
        case GIMAGE_JPG:
            return gimage_loadJpg(pathname, buf);
    }

    return GIMAGE_UNRECOGNIZED_FORMAT;
}

void gimage_premultiplyAlpha(int width, int height, void *buf)
{
    unsigned int *ibuf = (unsigned int *)buf;

    for (int i = 0; i < width * height; ++i)
        ibuf[i] = premultiply(ibuf[i]);
}

}
