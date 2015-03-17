#ifndef ADS_H
#define ADS_H

#include <gglobal.h>
#include <gevent.h>




enum
{
	GADS_AD_RECEIVED_EVENT,
	GADS_AD_DISPLAYED_EVENT,
	GADS_AD_FAILED_EVENT,
	GADS_AD_ACTION_BEGIN_EVENT,
	GADS_AD_ACTION_END_EVENT,
	GADS_AD_DISMISSED_EVENT,
	GADS_AD_ERROR_EVENT,
};

typedef struct gads_AdErrorEvent
{
	const char *ad;
	const char *error;
} gads_AdErrorEvent;

typedef struct gads_AdFailedEvent
{
	const char *ad;
	const char *type;
	const char *error;
} gads_AdFailedEvent;

typedef struct gads_SimpleEvent
{
	const char *ad;
	const char *type;
} gads_SimpleEvent;

typedef struct gads_Parameter
{
    const char *value;
} gads_Parameter;

#ifdef __cplusplus
extern "C" {
#endif
    
G_API int gads_isAvailable();

G_API void gads_init();
G_API void gads_cleanup();

G_API void gads_initialize(const char *ad);
G_API void gads_destroy(const char *ad);
G_API void gads_setKey(const char *ad, gads_Parameter *params);

G_API void gads_loadAd(const char *ad, gads_Parameter *params);
G_API void gads_showAd(const char *ad, gads_Parameter *params);
G_API void gads_hideAd(const char *ad, const char *type);
G_API void gads_enableTesting(const char *ad);
G_API void gads_setAlignment(const char *ad, const char *hor, const char *ver);

G_API void gads_setX(const char *ad, int x);
G_API void gads_setY(const char *ad, int y);
G_API int gads_getX(const char *ad);
G_API int gads_getY(const char *ad);
G_API int gads_getWidth(const char *ad);
G_API int gads_getHeight(const char *ad);

G_API g_id gads_addCallback(gevent_Callback callback, void *udata);
G_API void gads_removeCallback(gevent_Callback callback, void *udata);
G_API void gads_removeCallbackWithGid(g_id gid);

#ifdef __cplusplus
}
#endif

#endif