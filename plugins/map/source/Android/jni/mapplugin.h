#ifndef MAPPLUGIN_H
#define MAPPLUGIN_H

#include <gglobal.h>
#include <gevent.h>
#include <string>

typedef struct gmapplugin_DoubleEvent
{
	const char *iap;
	const char *value;
} gmapplugin_DoubleEvent;

typedef struct gmapplugin_SimpleEvent
{
	const char *iap;
} gmapplugin_SimpleEvent;

typedef struct gmapplugin_SimpleParam
{
  std::string value;
} gmapplugin_SimpleParam;

typedef struct gmapplugin_Parameter
{
    const char *value;
	size_t size;
} gmapplugin_Parameter;

typedef struct Product
{
	std::string productId;
	std::string title;
	std::string description;
	std::string price;
} Product;

typedef struct gmapplugin_Product
{
	const char* productId;
	const char* title;
	const char* description;
	const char* price;
} gmapplugin_Product;

typedef struct gmapplugin_Products
{
	const char* iap;
	int count;
	gmapplugin_Product *products;
} gmapplugin_Products;

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
G_API gmapplugin_SimpleParam* gmapplugin_detectStores(gmapplugin_SimpleParam *params);

G_API void gmapplugin_initialize();
G_API void gmapplugin_destroy(const char *iab);
/*
G_API void gmapplugin_setProducts(const char *iab, gmapplugin_DoubleEvent *params);
G_API void gmapplugin_setConsumables(const char *iab, const char * const *params);

G_API void gmapplugin_check(const char *iab);
G_API void gmapplugin_request(const char *iab);
G_API void gmapplugin_purchase(const char *iab, const char *productId);
G_API void gmapplugin_restore(const char *iab);
*/

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
G_API void gmapplugin_addMarkerAtIndex(const char *iab, double lat, double lon, const char *title, int index);
G_API void gmapplugin_setMarkerTitle(const char *iab, int idx, const char *title);
G_API void gmapplugin_setMarkerHue(const char *iab, int idx, double hue);
G_API void gmapplugin_setMarkerAlpha(const char *iab, int idx, double alpha);
G_API void gmapplugin_setMarkerCoordinates(const char *iab, int idx, double lat, double lon);
G_API const char *gmapplugin_getMarkerTitle(const char *iab, int idx);
G_API double gmapplugin_getMarkerLatitude(const char *iab, int idx);
G_API double gmapplugin_getMarkerLongitude(const char *iab, int idx);
G_API int gmapplugin_getClickedMarkerIndex(const char *iab);



G_API g_id gmapplugin_addCallback(gevent_Callback callback, void *udata);
G_API void gmapplugin_removeCallback(gevent_Callback callback, void *udata);
G_API void gmapplugin_removeCallbackWithGid(g_id gid);

#ifdef __cplusplus
}
#endif

#endif