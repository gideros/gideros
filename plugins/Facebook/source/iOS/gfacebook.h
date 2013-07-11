#ifndef GFACEBOOK_H
#define GFACEBOOK_H

#include <gglobal.h>
#include <gevent.h>
#include <string.h>
#include <time.h>

enum
{
    GFACEBOOK_LOGIN_COMPLETE_EVENT,
    GFACEBOOK_LOGIN_ERROR_EVENT,
    GFACEBOOK_LOGIN_CANCEL_EVENT,
    GFACEBOOK_LOGOUT_COMPLETE_EVENT,
    GFACEBOOK_DIALOG_COMPLETE_EVENT,
    GFACEBOOK_DIALOG_ERROR_EVENT,
    GFACEBOOK_DIALOG_CANCEL_EVENT,
    GFACEBOOK_REQUEST_COMPLETE_EVENT,
    GFACEBOOK_REQUEST_ERROR_EVENT,
};

typedef struct gfacebook_DialogErrorEvent
{
    int errorCode;
    const char *errorDescription;
} gfacebook_DialogErrorEvent;

typedef struct gfacebook_RequestCompleteEvent
{
    const char *response;
    size_t responseLength;
} gfacebook_RequestCompleteEvent;

typedef struct gfacebook_RequestErrorEvent
{
    int errorCode;
    const char *errorDescription;
} gfacebook_RequestErrorEvent;

typedef struct gfacebook_Parameter
{
    const char *key;
    const char *value;
} gfacebook_Parameter;


#ifdef __cplusplus
extern "C" {
#endif
    
G_API int gfacebook_isAvailable();

G_API void gfacebook_init();
G_API void gfacebook_cleanup();

G_API void gfacebook_setAppId(const char *appId);

G_API void gfacebook_authorize(const char * const *permissions);
G_API void gfacebook_logout();
G_API int gfacebook_isSessionValid();
    
G_API void gfacebook_dialog(const char *action, const gfacebook_Parameter *params);
G_API void gfacebook_graphRequest(const char *graphPath, const gfacebook_Parameter *params, const char *httpMethod);

G_API void gfacebook_setAccessToken(const char *accessToken);
G_API const char *gfacebook_getAccessToken();

G_API void gfacebook_setExpirationDate(time_t time);
G_API time_t gfacebook_getExpirationDate();

G_API void gfacebook_extendAccessToken();
G_API void gfacebook_extendAccessTokenIfNeeded();
G_API int gfacebook_shouldExtendAccessToken();

G_API g_id gfacebook_addCallback(gevent_Callback callback, void *udata);
G_API void gfacebook_removeCallback(gevent_Callback callback, void *udata);
G_API void gfacebook_removeCallbackWithGid(g_id gid);

#ifdef __cplusplus
}
#endif

#endif

