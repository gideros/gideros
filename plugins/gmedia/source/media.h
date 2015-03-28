#ifndef MEDIA_H
#define MEDIA_H

#include "gideros.h"

typedef struct gmedia_ReceivedEvent
{
    const char* path;
} gmedia_ReceivedEvent;

enum
{
    GMEDIA_RECEIVED_EVENT,
    GMEDIA_CANCELED_EVENT,
    GMEDIA_COMPLETED_EVENT
};


#ifdef __cplusplus
extern "C" {
#endif

G_API void gmedia_init();
G_API void gmedia_cleanup();

G_API int gmedia_isCameraAvailable();
G_API void gmedia_takePicture();
G_API void gmedia_takeScreenshot();
G_API void gmedia_getPicture();
G_API void gmedia_savePicture(const char* path);
G_API void gmedia_playVideo(const char* path, int force);

G_API g_id gmedia_addCallback(gevent_Callback callback, void *udata);
G_API void gmedia_removeCallback(gevent_Callback callback, void *udata);
G_API void gmedia_removeCallbackWithGid(g_id gid);

#ifdef __cplusplus
}
#endif

#endif
