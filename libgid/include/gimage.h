#ifndef _GIMAGE_H_
#define _GIMAGE_H_

#include <gglobal.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GIMAGE_NO_ERROR 0
#define GIMAGE_CANNOT_OPEN_FILE 1
#define GIMAGE_UNRECOGNIZED_FORMAT 2
#define GIMAGE_ERROR_WHILE_READING 3
#define GIMAGE_UNSUPPORTED_COLOR_SPACE 4
#define GIMAGE_ERROR_WHILE_WRITING 5

G_API int gimage_parsePng(const char *pathname, int *width, int *height, int *comp);
G_API int gimage_loadPng(const char *pathname, void *buf);
G_API int gimage_savePng(const char *pathname, int width, int height, unsigned char *data);

G_API int gimage_parseJpg(const char *pathname, int *width, int *height, int *comp);
G_API int gimage_loadJpg(const char *pathname, void *buf);
G_API int gimage_saveJpg(const char *pathname, int width, int height, unsigned char *data);

G_API int gimage_parseImage(const char *pathname, int *width, int *height, int *comp);
G_API int gimage_loadImage(const char *pathname, void *buf);
G_API int gimage_saveImage(const char *pathname, int width, int height, unsigned char *data);

G_API void gimage_premultiplyAlpha(int width, int height, void *buf);

//gimage_loadPngAsync
//gimage_loadJpgAsync
//gimage_tick

#ifdef __cplusplus
}
#endif

#endif
