#include <ggooglebilling.h>
#include <jni.h>
#include <stdlib.h>
#include <glog.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

class GGoogleBilling
{
public:
	GGoogleBilling()
	{
		gid_ = g_NextId();
		
		JNIEnv *env = g_getJNIEnv();

		jclass localClass = env->FindClass("com/giderosmobile/android/plugins/googlebilling/GGoogleBilling");
		cls_ = (jclass)env->NewGlobalRef(localClass);
		env->DeleteLocalRef(localClass);

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "init", "(J)V"), (jlong)this);
	}

	~GGoogleBilling()
	{
		JNIEnv *env = g_getJNIEnv();

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "cleanup", "()V"));
		
		env->DeleteGlobalRef(cls_);
		
		gevent_RemoveEventsWithGid(gid_);
	}
	
	void setPublicKey(const char *publicKey)
	{
		JNIEnv *env = g_getJNIEnv();
		
		jstring jpublicKey = env->NewStringUTF(publicKey);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setPublicKey", "(Ljava/lang/String;)V"), jpublicKey);
		env->DeleteLocalRef(jpublicKey);
	}
	
	void setApiVersion(int apiVersion)
	{
		JNIEnv *env = g_getJNIEnv();
	
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setApiVersion", "(I)V"), (jint)apiVersion);
	}
	
	bool checkBillingSupported(const char *productType)
	{
		JNIEnv *env = g_getJNIEnv();

		jstring jproductType = productType ? env->NewStringUTF(productType) : NULL;
		
		jboolean result = env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "checkBillingSupported", "(Ljava/lang/String;)Z"), jproductType);
		
		if (jproductType)
			env->DeleteLocalRef(jproductType);		
		
		return result;
	}
	
	bool requestPurchase(const char *productId, const char* productType, const char *developerPayload)
	{
		JNIEnv *env = g_getJNIEnv();
	
		jstring jproductId = env->NewStringUTF(productId);
		jstring jproductType = productType ? env->NewStringUTF(productType) : NULL;
		jstring jdeveloperPayload = developerPayload ? env->NewStringUTF(developerPayload) : NULL;
		
		jboolean result = env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "requestPurchase", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z"), jproductId, jproductType, jdeveloperPayload);
	
		env->DeleteLocalRef(jproductId);
		if (jproductType)
			env->DeleteLocalRef(jproductType);
		if (jdeveloperPayload)
			env->DeleteLocalRef(jdeveloperPayload);
		
		return result;
	}
	
	bool confirmNotification(const char *notificationId)
	{
		JNIEnv *env = g_getJNIEnv();
	
		jstring jnotificationId = env->NewStringUTF(notificationId);
	
		jboolean result = env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "confirmNotification", "(Ljava/lang/String;)Z"), jnotificationId);
	
		env->DeleteLocalRef(jnotificationId);		
		
		return result;
	}
	
	bool restoreTransactions()
	{
		JNIEnv *env = g_getJNIEnv();

		jboolean result = env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "restoreTransactions", "()Z"));

		return result;	
	}
	
	void onBillingSupported(jint responseCode, jstring jproductType)
	{
		JNIEnv *env = g_getJNIEnv();

		const char *productType = jproductType ? env->GetStringUTFChars(jproductType, NULL) : NULL;

		ggooglebilling_CheckBillingSupportedCompleteEvent *event = (ggooglebilling_CheckBillingSupportedCompleteEvent*)gevent_CreateEventStruct1(
			sizeof(ggooglebilling_CheckBillingSupportedCompleteEvent),
			offsetof(ggooglebilling_CheckBillingSupportedCompleteEvent, productType), productType);
	
		event->responseCode = responseCode;

		if (jproductType)
			env->ReleaseStringUTFChars(jproductType, productType);

		gevent_EnqueueEvent(gid_, callback_s, GGOOGLEBILLING_CHECK_BILLING_SUPPORTED_COMPLETE_EVENT, event, 1, this);
	}
	
	void onPurchaseStateChange(jint purchaseState, jstring jproductId, jstring jnotificationId, jlong purchaseTime, jstring jdeveloperPayload)
	{
		JNIEnv *env = g_getJNIEnv();

		const char *productId = env->GetStringUTFChars(jproductId, NULL);
		const char *notificationId = jnotificationId ? env->GetStringUTFChars(jnotificationId, NULL) : NULL;
		const char *developerPayload = jdeveloperPayload ? env->GetStringUTFChars(jdeveloperPayload, NULL) : NULL;
		
		ggooglebilling_PurchaseStateChangeEvent *event = (ggooglebilling_PurchaseStateChangeEvent*)gevent_CreateEventStruct3(
			sizeof(ggooglebilling_PurchaseStateChangeEvent),
			offsetof(ggooglebilling_PurchaseStateChangeEvent, productId), productId,
			offsetof(ggooglebilling_PurchaseStateChangeEvent, notificationId), notificationId,
			offsetof(ggooglebilling_PurchaseStateChangeEvent, developerPayload), developerPayload);
		
		event->purchaseState = purchaseState;
		event->purchaseTime = purchaseTime / 1000;
		
		env->ReleaseStringUTFChars(jproductId, productId);
		if (jnotificationId)
			env->ReleaseStringUTFChars(jnotificationId, notificationId);
		if (jdeveloperPayload)
			env->ReleaseStringUTFChars(jdeveloperPayload, developerPayload);

		gevent_EnqueueEvent(gid_, callback_s, GGOOGLEBILLING_PURCHASE_STATE_CHANGE_EVENT, event, 1, this);
	}
	
	void onRequestPurchaseResponse(jint responseCode, jstring jproductId, jstring jproductType, jstring jdeveloperPayload)
	{
		JNIEnv *env = g_getJNIEnv();

		const char *productId = env->GetStringUTFChars(jproductId, NULL);
		const char *productType = jproductType ? env->GetStringUTFChars(jproductType, NULL) : NULL;
		const char *developerPayload = jdeveloperPayload ? env->GetStringUTFChars(jdeveloperPayload, NULL) : NULL;
		
		ggooglebilling_RequestPurchaseCompleteEvent *event = (ggooglebilling_RequestPurchaseCompleteEvent*)gevent_CreateEventStruct3(
			sizeof(ggooglebilling_RequestPurchaseCompleteEvent),
			offsetof(ggooglebilling_RequestPurchaseCompleteEvent, productId), productId,
			offsetof(ggooglebilling_RequestPurchaseCompleteEvent, productType), productType,
			offsetof(ggooglebilling_RequestPurchaseCompleteEvent, developerPayload), developerPayload);
			
		event->responseCode = responseCode;
	
		env->ReleaseStringUTFChars(jproductId, productId);
		if (jproductType)
			env->ReleaseStringUTFChars(jproductType, productType);
		if (jdeveloperPayload)
			env->ReleaseStringUTFChars(jdeveloperPayload, developerPayload);
		
		gevent_EnqueueEvent(gid_, callback_s, GGOOGLEBILLING_REQUEST_PURCHASE_COMPLETE_EVENT, event, 1, this);
	}
	
	void onRestoreTransactionsResponse(jint responseCode)
	{
		JNIEnv *env = g_getJNIEnv();

		ggooglebilling_RestoreTransactionsCompleteEvent *event = (ggooglebilling_RestoreTransactionsCompleteEvent*)malloc(sizeof(ggooglebilling_RestoreTransactionsCompleteEvent));
		
		event->responseCode = responseCode;
		
		gevent_EnqueueEvent(gid_, callback_s, GGOOGLEBILLING_RESTORE_TRANSACTIONS_COMPLETE_EVENT, event, 1, this);
	}
	
	void onConfirmNotificationsResponse(jint responseCode, jstring jnotificationId)
	{
		JNIEnv *env = g_getJNIEnv();

		const char *notificationId = env->GetStringUTFChars(jnotificationId, NULL);

		ggooglebilling_ConfirmNotificationCompleteEvent *event = (ggooglebilling_ConfirmNotificationCompleteEvent*)gevent_CreateEventStruct1(
			sizeof(ggooglebilling_ConfirmNotificationCompleteEvent),
			offsetof(ggooglebilling_ConfirmNotificationCompleteEvent, notificationId), notificationId);
		
		event->responseCode = responseCode;

		env->ReleaseStringUTFChars(jnotificationId, notificationId);

		gevent_EnqueueEvent(gid_, callback_s, GGOOGLEBILLING_CONFIRM_NOTIFICATION_COMPLETE_EVENT, event, 1, this);
	}
	
	g_id addCallback(gevent_Callback callback, void *udata)
	{
		return callbackList_.addCallback(callback, udata);
	}
	void removeCallback(gevent_Callback callback, void *udata)
	{
		callbackList_.removeCallback(callback, udata);
	}
	void removeCallbackWithGid(g_id gid)
	{
		callbackList_.removeCallbackWithGid(gid);
	}

private:
	static void callback_s(int type, void *event, void *udata)
	{
		((GGoogleBilling*)udata)->callback(type, event);
	}

	void callback(int type, void *event)
	{
		callbackList_.dispatchEvent(type, event);
	}

private:
	gevent_CallbackList callbackList_;

private:
	jclass cls_;
	g_id gid_;
};

extern "C" {

void Java_com_giderosmobile_android_plugins_googlebilling_GGoogleBilling_onBillingSupported(JNIEnv *env, jclass clz, jint responseCode, jstring type, jlong data)
{
	((GGoogleBilling*)data)->onBillingSupported(responseCode, type);
}

void Java_com_giderosmobile_android_plugins_googlebilling_GGoogleBilling_onPurchaseStateChange(JNIEnv *env, jclass clz, jint purchaseState, jstring productId, jstring notificationId, jlong purchaseTime, jstring developerPayload, jlong data)
{
	((GGoogleBilling*)data)->onPurchaseStateChange(purchaseState, productId, notificationId, purchaseTime, developerPayload);
}

void Java_com_giderosmobile_android_plugins_googlebilling_GGoogleBilling_onRequestPurchaseResponse(JNIEnv *env, jclass clz, jint responseCode, jstring productId, jstring productType, jstring developerPayload, long data)
{
	((GGoogleBilling*)data)->onRequestPurchaseResponse(responseCode, productId, productType, developerPayload);
}

void Java_com_giderosmobile_android_plugins_googlebilling_GGoogleBilling_onRestoreTransactionsResponse(JNIEnv *env, jclass clz, jint responseCode, long data)
{
	((GGoogleBilling*)data)->onRestoreTransactionsResponse(responseCode);
}

void Java_com_giderosmobile_android_plugins_googlebilling_GGoogleBilling_onConfirmNotificationsResponse(JNIEnv *env, jclass clz, jint responseCode, jstring notificationId, long data)
{
	((GGoogleBilling*)data)->onConfirmNotificationsResponse(responseCode, notificationId);
}

}

static GGoogleBilling *s_googlebilling = NULL;

extern "C" {

int ggooglebilling_isAvailable()
{
	return 1;
}

void ggooglebilling_init()
{
	s_googlebilling = new GGoogleBilling;
}

void ggooglebilling_cleanup()
{
	delete s_googlebilling;
	s_googlebilling = NULL;
}

void ggooglebilling_setPublicKey(const char *publicKey)
{
	s_googlebilling->setPublicKey(publicKey);
}

void ggooglebilling_setApiVersion(int apiVersion)
{
	s_googlebilling->setApiVersion(apiVersion);
}

int ggooglebilling_checkBillingSupported(const char *productType)
{
	return s_googlebilling->checkBillingSupported(productType);
}

int ggooglebilling_requestPurchase(const char *productId, const char *productType, const char *developerPayload)
{
	return s_googlebilling->requestPurchase(productId, productType, developerPayload);
}

int ggooglebilling_confirmNotification(const char *notificationId)
{
	return s_googlebilling->confirmNotification(notificationId);
}

int ggooglebilling_restoreTransactions()
{
	return s_googlebilling->restoreTransactions();
}

g_id ggooglebilling_addCallback(gevent_Callback callback, void *udata)
{
	return s_googlebilling->addCallback(callback, udata);
}

void ggooglebilling_removeCallback(gevent_Callback callback, void *udata)
{
	s_googlebilling->removeCallback(callback, udata);
}

void ggooglebilling_removeCallbackWithGid(g_id gid)
{
	s_googlebilling->removeCallbackWithGid(gid);
}

}
