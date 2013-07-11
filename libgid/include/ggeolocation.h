#ifndef _GGEOLOCATION_H_
#define _GGEOLOCATION_H_

#include <stdlib.h>
#include <gglobal.h>
#include <gevent.h>

typedef struct ggeolocation_LocationUpdateEvent
{
    double latitude;
    double longitude;
    double altitude;
} ggeolocation_LocationUpdateEvent;

typedef struct ggeolocation_HeadingUpdateEvent
{
    double magneticHeading;
    double trueHeading;
} ggeolocation_HeadingUpdateEvent;

typedef struct ggeolocation_ErrorEvent
{
} ggeolocation_ErrorEvent;

#ifdef __cplusplus
extern "C" {
#endif

G_API void ggeolocation_init();
G_API void ggeolocation_cleanup();

G_API int ggeolocation_isAvailable();
G_API int ggeolocation_isHeadingAvailable();

G_API void ggeolocation_setAccuracy(double accuracy);
G_API double ggeolocation_getAccuracy();

G_API void ggeolocation_setThreshold(double threshold);
G_API double ggeolocation_getThreshold();

// recursive
G_API void ggeolocation_startUpdatingLocation();

// recursive
G_API void ggeolocation_stopUpdatingLocation();

// recursive
G_API void ggeolocation_startUpdatingHeading();

// recursive
G_API void ggeolocation_stopUpdatingHeading();

G_API g_id ggeolocation_addCallback(gevent_Callback callback, void *udata);
G_API void ggeolocation_removeCallback(gevent_Callback callback, void *udata);
G_API void ggeolocation_removeCallbackWithGid(g_id gid);

#ifdef __cplusplus
}
#endif

#endif
