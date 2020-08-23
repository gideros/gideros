#ifndef IAB_H
#define IAB_H

#include <gglobal.h>
#include <gevent.h>
#include <string>

enum
{
	GIAB_AVAILABLE_EVENT,
	GIAB_NOT_AVAILABLE_EVENT,
	GIAB_PURCHASE_COMPLETE_EVENT,
	GIAB_PURCHASE_ERROR_EVENT,
	GIAB_PRODUCTS_COMPLETE_EVENT,
	GIAB_PRODUCTS_ERROR_EVENT,
	GIAB_RESTORE_COMPLETE_EVENT,
	GIAB_RESTORE_ERROR_EVENT,
};

typedef struct giab_DoubleEvent
{
	const char *iap;
	const char *value;
} giab_DoubleEvent;

typedef struct giab_SimpleEvent
{
	const char *iap;
} giab_SimpleEvent;

typedef struct giab_SimpleParam
{
    std::string value;
} giab_SimpleParam;

typedef struct giab_Parameter
{
    const char *value;
	size_t size;
} giab_Parameter;

typedef struct Product
{
	std::string productId;
	std::string title;
	std::string description;
	std::string price;
} Product;

typedef struct giab_Product
{
	const char* productId;
	const char* title;
	const char* description;
	const char* price;
} giab_Product;

typedef struct giab_Products
{
	const char* iap;
	int count;
	giab_Product *products;
} giab_Products;

typedef struct giab_Purchase
{
	const char* iap;
	const char* productId;
	const char* receiptId;
} giab_Purchase;

#ifdef __cplusplus
extern "C" {
#endif

G_API void giab_init();
G_API void giab_cleanup();
G_API giab_SimpleParam* giab_detectStores(giab_SimpleParam *params);

G_API void giab_initialize(const char *iab);
G_API void giab_destroy(const char *iab);
G_API void giab_setup(const char *iab, giab_Parameter *params);
G_API void giab_setProducts(const char *iab, giab_DoubleEvent *params);
G_API void giab_setConsumables(const char *iab, const char * const *params);

G_API void giab_check(const char *iab);
G_API void giab_request(const char *iab);
G_API void giab_purchase(const char *iab, const char *productId);
G_API void giab_restore(const char *iab);
    
G_API void giab_onAvailable(const char *iap);
G_API void giab_onNotAvailable(const char *iap);
G_API void giab_onPurchaseComplete(const char* iap, const char *productId, const char* receiptId);
G_API void giab_onPurchaseError(const char *iap, const char *value);
G_API void giab_onProductsComplete(const char *iab, Product *product);
G_API void giab_onProductsError(const char *iap, const char *value);
G_API void giab_onRestoreComplete(const char *iap);
G_API void giab_onRestoreError(const char *iap, const char *value);

G_API g_id giab_addCallback(gevent_Callback callback, void *udata);
G_API void giab_removeCallback(gevent_Callback callback, void *udata);
G_API void giab_removeCallbackWithGid(g_id gid);

#ifdef __cplusplus
}
#endif

#endif