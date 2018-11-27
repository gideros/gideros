#ifndef MAPPLUGIN_H
#define MAPPLUGIN_H

#include "gglobal.h"
//#include <gevent.h>
#include "string.h"

typedef struct gmapplugin_DoubleEvent
{
	const char *iap;
	const char *value;
} gmapplugin_DoubleEvent;

typedef struct gmapplugin_SimpleEvent
{
	const char *iap;
} gmapplugin_SimpleEvent;


/*
typedef struct gmapplugin_SimpleParam
{
    std::string value;
} gmapplugin_SimpleParam;
*/

typedef struct gmapplugin_Parameter
{
    const char *value;
	size_t size;
} gmapplugin_Parameter;

typedef struct gmapplugin_Purchase
{
	const char* iap;
	const char* productId;
	const char* receiptId;
} gmapplugin_Purchase;

#ifdef __cplusplus
extern "C" {
#endif

G_API void gmapplugin_init();
G_API void gmapplugin_cleanup();

G_API void gmapplugin_initialize(const char *iab);
G_API void gmapplugin_destroy(const char *iab);
G_API void gmapplugin_setup(const char *iab, gmapplugin_Parameter *params);

G_API void gmapplugin_setDimensions(const char *iab, int width, int height);
G_API void gmapplugin_setPosition(const char *iab, int x, int y);
G_API void gmapplugin_hide(const char *iab);
G_API void gmapplugin_show(const char *iab);
G_API void gmapplugin_clear(const char *iab);
G_API void gmapplugin_setCenterCoordinates(const char *iab, double lat, double lon);
G_API void gmapplugin_setZoom(const char *iab, int z);
G_API int gmapplugin_mapClicked(const char *iab);
G_API double gmapplugin_getMapClickLatitude(const char *iab);
G_API double gmapplugin_getMapClickLongitude(const char *iab);
G_API void gmapplugin_setType(const char *iab, int t);
G_API double gmapplugin_getCenterLatitude(const char *iab);
G_API double gmapplugin_getCenterLongitude(const char *iab);
G_API int gmapplugin_addMarker(const char *iab, double lat, double lon, const char *title);
G_API void gmapplugin_setMarkerTitle(const char *iab, int idx, const char *title);
G_API void gmapplugin_setMarkerHue(const char *iab, int idx, double hue);
G_API void gmapplugin_setMarkerAlpha(const char *iab, int idx, double alpha);
G_API void gmapplugin_setMarkerCoordinates(const char *iab, int idx, double lat, double lon);
G_API const char *gmapplugin_getMarkerTitle(const char *iab, int idx);
G_API double gmapplugin_getMarkerLatitude(const char *iab, int idx);
G_API double gmapplugin_getMarkerLongitude(const char *iab, int idx);
G_API int gmapplugin_getClickedMarkerIndex(const char *iab);



//G_API g_id gmapplugin_addCallback(gevent_Callback callback, void *udata);
//G_API void gmapplugin_removeCallback(gevent_Callback callback, void *udata);
//G_API void gmapplugin_removeCallbackWithGid(g_id gid);

#ifdef __cplusplus
}
#endif

#endif