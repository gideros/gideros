#ifndef GHTTP_H
#define GHTTP_H

#include <stdlib.h>
#include <gevent.h>

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    GHTTP_RESPONSE_EVENT,
    GHTTP_ERROR_EVENT,
    GHTTP_PROGRESS_EVENT,
};

typedef struct ghttp_Header {
    const char *name;
    const char *value;
} ghttp_Header;

typedef struct ghttp_ResponseEvent
{
    void *data;
    size_t size;
    int httpStatusCode;
    ghttp_Header headers[1];
} ghttp_ResponseEvent;

typedef struct ghttp_ErrorEvent
{
} ghttp_ErrorEvent;

typedef struct ghttp_ProgressEvent
{
    size_t bytesLoaded;
    size_t bytesTotal;
} ghttp_ProgressEvent;

G_API void ghttp_Init();
G_API void ghttp_Cleanup();

G_API void ghttp_IgnoreSSLErrors();
G_API void ghttp_SetProxy(const char *host, int port, const char *user, const char *pass);
G_API g_id ghttp_Get(const char *url, const ghttp_Header *headers, gevent_Callback callback, void *udata);
G_API g_id ghttp_Post(const char *url, const ghttp_Header *headers, const void *data, size_t size, gevent_Callback callback, void *udata);
G_API g_id ghttp_Delete(const char *url, const ghttp_Header *headers, gevent_Callback callback, void *udata);
G_API g_id ghttp_Put(const char *url, const ghttp_Header *headers, const void *data, size_t size, gevent_Callback callback, void *udata);
G_API void ghttp_Close(g_id id);
G_API void ghttp_CloseAll();

#ifdef __cplusplus
}
#endif

#endif
