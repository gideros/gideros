#ifndef GGOOGLEBILLING_H
#define GGOOGLEBILLING_H

#include <gglobal.h>
#include <gevent.h>
#include <time.h>

enum
{
	GGOOGLEBILLING_RESULT_OK = 0,
	GGOOGLEBILLING_RESULT_USER_CANCELED = 1,
	GGOOGLEBILLING_RESULT_SERVICE_UNAVAILABLE = 2,
	GGOOGLEBILLING_RESULT_BILLING_UNAVAILABLE = 3,
	GGOOGLEBILLING_RESULT_ITEM_UNAVAILABLE = 4,
	GGOOGLEBILLING_RESULT_DEVELOPER_ERROR = 5,
	GGOOGLEBILLING_RESULT_ERROR = 6,
};

enum
{
	GGOOGLEBILLING_PURCHASED = 0,
	GGOOGLEBILLING_CANCELED = 1,
	GGOOGLEBILLING_REFUNDED = 2,
	GGOOGLEBILLING_EXPIRED = 3,
};

enum
{
	GGOOGLEBILLING_CHECK_BILLING_SUPPORTED_COMPLETE_EVENT,
	GGOOGLEBILLING_REQUEST_PURCHASE_COMPLETE_EVENT,
	GGOOGLEBILLING_RESTORE_TRANSACTIONS_COMPLETE_EVENT,
	GGOOGLEBILLING_CONFIRM_NOTIFICATION_COMPLETE_EVENT,
	GGOOGLEBILLING_PURCHASE_STATE_CHANGE_EVENT,
};

typedef struct ggooglebilling_CheckBillingSupportedCompleteEvent
{
	int responseCode;
	const char *productType;
} ggooglebilling_CheckBillingSupportedCompleteEvent;

typedef struct ggooglebilling_PurchaseStateChangeEvent
{
	int purchaseState;
	const char *productId;
	const char *notificationId;
	time_t purchaseTime;
	const char *developerPayload;
} ggooglebilling_PurchaseStateChangeEvent;

typedef struct ggooglebilling_RequestPurchaseCompleteEvent
{
	int responseCode;
	const char *productId;
	const char *productType;
	const char *developerPayload;
} ggooglebilling_RequestPurchaseCompleteEvent;

typedef struct ggooglebilling_RestoreTransactionsCompleteEvent
{
	int responseCode;
} ggooglebilling_RestoreTransactionsCompleteEvent;

typedef struct ggooglebilling_ConfirmNotificationCompleteEvent
{
	int responseCode;
	const char *notificationId;
} ggooglebilling_ConfirmNotificationCompleteEvent;

#ifdef __cplusplus
extern "C" {
#endif
    
G_API int ggooglebilling_isAvailable();

G_API void ggooglebilling_init();
G_API void ggooglebilling_cleanup();

G_API void ggooglebilling_setPublicKey(const char *publicKey);

G_API void ggooglebilling_setApiVersion(int apiVersion);

G_API int ggooglebilling_checkBillingSupported(const char *productType);
G_API int ggooglebilling_requestPurchase(const char *productId, const char *productType, const char *developerPayload);
G_API int ggooglebilling_confirmNotification(const char *notificationId);
G_API int ggooglebilling_restoreTransactions();

G_API g_id ggooglebilling_addCallback(gevent_Callback callback, void *udata);
G_API void ggooglebilling_removeCallback(gevent_Callback callback, void *udata);
G_API void ggooglebilling_removeCallbackWithGid(g_id gid);

#ifdef __cplusplus
}
#endif

#endif