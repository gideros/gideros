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
    GFACEBOOK_LOGOUT_COMPLETE_EVENT,
    GFACEBOOK_LOGOUT_ERROR_EVENT,
    GFACEBOOK_OPEN_URL_EVENT,
    GFACEBOOK_DIALOG_COMPLETE_EVENT,
    GFACEBOOK_DIALOG_ERROR_EVENT,
    GFACEBOOK_REQUEST_COMPLETE_EVENT,
    GFACEBOOK_REQUEST_ERROR_EVENT,
};

typedef struct gfacebook_SimpleEvent
{
    const char *value;
} gfacebook_SimpleEvent;

typedef struct gfacebook_DoubleEvent
{
    const char *type;
    const char *value;
} gfacebook_DoubleEvent;

typedef struct gfacebook_ResponseEvent
{
    const char *type;
    const char *response;
    size_t responseLength;
} gfacebook_ResponseEvent;

typedef struct gfacebook_Parameter
{
    const char *key;
    const char *value;
} gfacebook_Parameter;


#ifdef __cplusplus
extern "C" {
#endif

G_API void gfacebook_init();
G_API void gfacebook_cleanup();

G_API void gfacebook_login(const char *appId, const char * const *permissions);
G_API void gfacebook_logout();
G_API const char* gfacebook_getAccessToken();
G_API time_t gfacebook_getExpirationDate();

G_API void gfacebook_upload(const char *path, const char *orig);    
G_API void gfacebook_dialog(const char *action, const gfacebook_Parameter *params);
G_API void gfacebook_request(const char *graphPath, const gfacebook_Parameter *params, int method);
    
G_API void gfacebook_onLoginComplete();
G_API void gfacebook_onLoginError(const char* value);
G_API void gfacebook_onLogoutComplete();
G_API void gfacebook_onLogoutError(const char* value);
G_API void gfacebook_onDialogComplete(const char* type, const char* value);
G_API void gfacebook_onDialogError(const char* type, const char* value);
G_API void gfacebook_onRequestError(const char* type, const char* value);
G_API void gfacebook_onRequestComplete(const char* type, const char* response);

G_API g_id gfacebook_addCallback(gevent_Callback callback, void *udata);
G_API void gfacebook_removeCallback(gevent_Callback callback, void *udata);
G_API void gfacebook_removeCallbackWithGid(g_id gid);

#ifdef __cplusplus
}
#endif

#endif

