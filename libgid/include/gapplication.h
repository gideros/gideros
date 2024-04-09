#ifndef _GAPPLICATION_H_
#define _GAPPLICATION_H_

#include <gglobal.h>
#include <gevent.h>

typedef struct gapplication_OpenUrlEvent
{
    const char *url;
} gapplication_OpenUrlEvent;

typedef struct gapplication_TextInputEvent
{
    const char *text;
    const char *context;
    int selStart;
    int selEnd;
} gapplication_TextInputEvent;

typedef enum gapplication_Orientation
{
    GAPPLICATION_PORTRAIT,
    GAPPLICATION_LANDSCAPE_LEFT,
    GAPPLICATION_PORTRAIT_UPSIDE_DOWN,
    GAPPLICATION_LANDSCAPE_RIGHT,
} gapplication_Orientation;

typedef enum gapplication_AutoOrientation
{
    GAPPLICATION_AUTO_ORIENTATION_NORMAL,
    GAPPLICATION_AUTO_ORIENTATION_DUAL,
    GAPPLICATION_AUTO_ORIENTATION_ALL,
    GAPPLICATION_AUTO_ORIENTATION_FIXED,
} gapplication_AutoOrientation;

typedef struct gapplication_OrientationChangeEvent
{
    gapplication_Orientation orientation;
} gapplication_OrientationChangeEvent;

typedef struct gapplication_PermissionEvent
{
	int count;
    const char **perms;
    int *status;
} gapplication_PermissionEvent;

#ifdef __cplusplus
extern "C" {
#endif

G_API void gapplication_init();
G_API void gapplication_cleanup();

G_API void gapplication_exit();

G_API int gapplication_getScreenDensity(int *ldpi);
G_API void gapplication_requestDeviceOrientation(gapplication_Orientation iO,gapplication_AutoOrientation iAutoRot);
G_API g_id gapplication_addCallback(gevent_Callback callback, void *udata);
G_API void gapplication_removeCallback(gevent_Callback callback, void *udata);
G_API void gapplication_removeCallbackWithGid(g_id gid);

G_API void gapplication_clipboardCallback(int luaFuncRef,int result,const char *data, const char *type);
G_API void gapplication_enqueueEvent(int type, void *event, int free);

#ifdef __cplusplus
}
#endif

#endif

