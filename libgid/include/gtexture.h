#ifndef _GTEXTURE_H_
#define _GTEXTURE_H_

#include <gglobal.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* format */
#define GTEXTURE_ALPHA 0
#define GTEXTURE_RGB 1
#define GTEXTURE_RGBA 2
#define GTEXTURE_LUMINANCE 3
#define GTEXTURE_LUMINANCE_ALPHA 4

/* type */
#define GTEXTURE_UNSIGNED_BYTE 0
#define GTEXTURE_UNSIGNED_SHORT_5_6_5 1
#define GTEXTURE_UNSIGNED_SHORT_4_4_4_4 2
#define GTEXTURE_UNSIGNED_SHORT_5_5_5_1 3

/* wrap */
#define GTEXTURE_REPEAT 0
#define GTEXTURE_CLAMP 1

/* filter */
#define GTEXTURE_NEAREST 0
#define GTEXTURE_LINEAR 1

G_API void gtexture_init();
G_API void gtexture_cleanup();

G_API g_id gtexture_create(int width, int height,
                           int format, int type,
                           int wrap, int filter,
                           const void *pixels,
                           const void *signature, size_t siglength);

G_API g_id gtexture_reuse(int format, int type,
                          int wrap, int filter,
                          const void *signature, size_t siglength);

G_API g_bool gtexture_delete(g_id id);
G_API unsigned int gtexture_getInternalId(g_id id);
G_API void gtexture_setUserData(g_id id, void *udata);
G_API void *gtexture_getUserData(g_id id);
G_API void gtexture_tick();
G_API void gtexture_setCachingEnabled(int caching);
G_API void gtexture_reloadTextures();

G_API size_t gtexture_getMemoryUsage();

G_API g_id gtexture_RenderTargetCreate(int width, int height,
                                       int wrap, int filter);

G_API unsigned int gtexture_RenderTargetGetFBO(g_id renderTarget);


G_API void gtexture_SaveRenderTargets();
G_API void gtexture_RestoreRenderTargets();

G_API g_id gtexture_TempTextureCreate(int width, int height);
G_API void gtexture_TempTextureDelete(g_id id);
G_API unsigned int gtexture_TempTextureGetName(g_id id);
G_API void gtexture_RestoreTempTextures();

//void gtexture_replace(g_id oldId, int format, int width, int height, int type, void *pixels);

#ifdef __cplusplus
}
#endif

#endif
