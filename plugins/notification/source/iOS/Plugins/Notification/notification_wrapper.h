#ifndef NOTIFICATION_WRAPPER_H
#define NOTIFICATION_WRAPPER_H

#include <gglobal.h>
#include <gevent.h>
#include "gideros.h"
#include <vector>
#include <glog.h>

enum
{
	NOTIFICATION_LOCAL_EVENT,
	NOTIFICATION_PUSH_EVENT,
	NOTIFICATION_PUSH_REGISTER_EVENT,
	NOTIFICATION_PUSH_REGISTER_ERROR_EVENT,
};

typedef struct gnotification_LocalEvent
{
	int id;
	const char *title;
	const char *text;
	int number;
	const char *sound;
    const char *custom;
    bool didOpen;
} gnotification_LocalEvent;

typedef struct gnotification_PushEvent
{
	int id;
	const char *title;
	const char *text;
	int number;
	const char *sound;
    const char *custom;
    bool didOpen;
} gnotification_PushEvent;

typedef struct gnotification_RegisterPushEvent
{
	const char *regId;
} gnotification_RegisterPushEvent;

typedef struct gnotification_RegisterPushErrorEvent
{
	const char *errorId;
} gnotification_RegisterPushErrorEvent;

typedef struct gnotification_Parameter
{
    const char *key;
    const char *value;
} gnotification_Parameter;

typedef struct gnotification_Group
{
	int id;
    const char *title;
    const char *message;
    int number;
	const char *sound;
    const char *custom;
} gnotification_Group;


#ifdef __cplusplus
extern "C" {
#endif

G_API void gnotification_construct();
G_API void gnotification_destroy();

G_API void gnotification_init(int id);
G_API void gnotification_cleanup(int id);

G_API void gnotification_set_title(int id, const char *title);
G_API const char* gnotification_get_title(int id);
G_API void gnotification_set_body(int id, const char *body);
G_API const char* gnotification_get_body(int id);
G_API void gnotification_set_number(int id, int number);
G_API int gnotification_get_number(int id);
G_API void gnotification_set_sound(int id, const char *sound);
G_API const char* gnotification_get_sound(int id);
G_API void gnotification_set_custom(int id, const char *custom);
G_API const char* gnotification_get_custom(int id);
G_API void gnotification_dispatch_now(int id);
G_API void gnotification_dispatch_after(int id, gnotification_Parameter *params1, gnotification_Parameter *params2);
G_API void gnotification_dispatch_on(int id, gnotification_Parameter *params1, gnotification_Parameter *params2);
G_API void gnotification_cancel(int id);
G_API void gnotification_cancel_all();

G_API void gnotification_clear_local();
G_API void gnotification_clear_push();
G_API gnotification_Group* gnotification_get_scheduled();
G_API gnotification_Group* gnotification_get_local();
G_API gnotification_Group* gnotification_get_push();
G_API void gnotification_register_push(const char *project);
G_API void gnotification_unregister_push();

G_API g_id gnotification_addCallback(gevent_Callback callback, void *udata);
G_API void gnotification_removeCallback(gevent_Callback callback, void *udata);
G_API void gnotification_removeCallbackWithGid(g_id gid);

G_API void gnotification_onLocalNotification(int nid, const char *title, const char *text, int number, const char *sound, const char* custom, bool didOpen);
G_API void gnotification_onPushNotification(int nid, const char *title, const char *text, int number, const char *sound, const char* custom, bool didOpen);
G_API void gnotification_onPushRegistration(const char* token);
G_API void gnotification_onPushRegistrationError(const char* error);

#ifdef __cplusplus
}
#endif

#endif