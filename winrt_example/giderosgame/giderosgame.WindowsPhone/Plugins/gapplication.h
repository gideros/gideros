#ifndef _GAPPLICATION_H_
#define _GAPPLICATION_H_

#include <gglobal.h>
#include <gevent.h>

typedef struct gapplication_OpenUrlEvent
{
    const char *url;
} gapplication_OpenUrlEvent;

typedef enum gapplication_Orientation
{
    GAPPLICATION_PORTRAIT,
    GAPPLICATION_LANDSCAPE_LEFT,
    GAPPLICATION_PORTRAIT_UPSIDE_DOWN,
    GAPPLICATION_LANDSCAPE_RIGHT,
} gapplication_Orientation;

typedef struct gapplication_OrientationChangeEvent
{
    gapplication_Orientation orientation;
} gapplication_OrientationChangeEvent;

#ifdef __cplusplus
extern "C" {
#endif

G_API void gapplication_init();
G_API void gapplication_cleanup();

G_API void gapplication_exit();

G_API int gapplication_getScreenDensity();

G_API g_id gapplication_addCallback(gevent_Callback callback, void *udata);
G_API void gapplication_removeCallback(gevent_Callback callback, void *udata);
G_API void gapplication_removeCallbackWithGid(g_id gid);

#ifdef __cplusplus
}
#endif

#endif

