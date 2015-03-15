#include <ads.h>
#include <jni.h>
#include <stdlib.h>
#include <glog.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

class GAds
{
public:
	GAds()
	{
		gid_ = g_NextId();
		
		JNIEnv *env = g_getJNIEnv();

		jclass localClass = env->FindClass("com/giderosmobile/android/plugins/ads/Ads");
		cls_ = (jclass)env->NewGlobalRef(localClass);
		env->DeleteLocalRef(localClass);
		
		jclass class_sparse = env->FindClass("android/util/SparseArray");
		clsSparse = static_cast<jclass>(env->NewGlobalRef(class_sparse));
		env->DeleteLocalRef(class_sparse);

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "init", "(J)V"), (jlong)this);
	}

	~GAds()
	{
		JNIEnv *env = g_getJNIEnv();

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "cleanup", "()V"));
		
		env->DeleteGlobalRef(cls_);
		env->DeleteGlobalRef(clsSparse);
		
		gevent_RemoveEventsWithGid(gid_);
	}
	
	void init(const char *ad)
	{
		JNIEnv *env = g_getJNIEnv();
		
		jstring jAd = env->NewStringUTF(ad);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "initialize", "(Ljava/lang/String;)V"), jAd);
		env->DeleteLocalRef(jAd);
	}
	
	void destroy(const char *ad)
	{
		JNIEnv *env = g_getJNIEnv();
		
		jstring jAd = env->NewStringUTF(ad);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "destroy", "(Ljava/lang/String;)V"), jAd);
		env->DeleteLocalRef(jAd);
	}
	
	void setKey(const char *ad, gads_Parameter *params)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jAd = env->NewStringUTF(ad);
		//create Java object
		jobject jparams = env->NewObject(clsSparse, env->GetMethodID(clsSparse, "<init>", "()V"));
		int i = 0;
		while (params->value)
		{
			jstring jVal = env->NewStringUTF(params->value);
			env->CallVoidMethod(jparams, env->GetMethodID(clsSparse, "put", "(ILjava/lang/Object;)V"), (jint)i, jVal);
			env->DeleteLocalRef(jVal);
			++params;
			i++;
		}
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setKey", "(Ljava/lang/String;Ljava/lang/Object;)V"), jAd, jparams);
		env->DeleteLocalRef(jparams);
		env->DeleteLocalRef(jAd);
	}
	
	void loadAd(const char *ad, gads_Parameter *params)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jAd = env->NewStringUTF(ad);
		//create Java object
		jobject jparams = env->NewObject(clsSparse, env->GetMethodID(clsSparse, "<init>", "()V"));
		int i = 0;
		while (params->value)
		{
			jstring jVal = env->NewStringUTF(params->value);
			env->CallVoidMethod(jparams, env->GetMethodID(clsSparse, "put", "(ILjava/lang/Object;)V"), (jint)i, jVal);
			env->DeleteLocalRef(jVal);
			++params;
			i++;
		}
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "loadAd", "(Ljava/lang/String;Ljava/lang/Object;)V"), jAd, jparams);
		env->DeleteLocalRef(jparams);
		env->DeleteLocalRef(jAd);
	}
	
	void showAd(const char *ad, gads_Parameter *params)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jAd = env->NewStringUTF(ad);
		//create Java object
		jobject jparams = env->NewObject(clsSparse, env->GetMethodID(clsSparse, "<init>", "()V"));
		int i = 0;
		while (params->value)
		{
			jstring jVal = env->NewStringUTF(params->value);
			env->CallVoidMethod(jparams, env->GetMethodID(clsSparse, "put", "(ILjava/lang/Object;)V"), (jint)i, jVal);
			env->DeleteLocalRef(jVal);
			++params;
			i++;
		}
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "showAd", "(Ljava/lang/String;Ljava/lang/Object;)V"), jAd, jparams);
		env->DeleteLocalRef(jparams);
		env->DeleteLocalRef(jAd);
	}
	
	void hideAd(const char *ad, const char *type)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jAd = env->NewStringUTF(ad);
		jstring jAdType = env->NewStringUTF(type);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "hideAd", "(Ljava/lang/String;Ljava/lang/String;)V"), jAd, jAdType);
		env->DeleteLocalRef(jAd);
		env->DeleteLocalRef(jAdType);
	}
	
	void enableTesting(const char *ad)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jAd = env->NewStringUTF(ad);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "enableTesting", "(Ljava/lang/String;)V"), jAd);
		env->DeleteLocalRef(jAd);
	}
	
	void setAlignment(const char *ad, const char *hor, const char *ver)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jAd = env->NewStringUTF(ad);
		jstring jHor = env->NewStringUTF(hor);
		jstring jVer = env->NewStringUTF(ver);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setAlignment", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V"), jAd, jHor, jVer);
		env->DeleteLocalRef(jHor);
		env->DeleteLocalRef(jVer);
		env->DeleteLocalRef(jAd);
	}
	
	void setX(const char *ad, int x)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jAd = env->NewStringUTF(ad);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setX", "(Ljava/lang/String;I)V"), jAd, (jint)x);
		env->DeleteLocalRef(jAd);
	}
	
	void setY(const char *ad, int y)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jAd = env->NewStringUTF(ad);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setY", "(Ljava/lang/String;I)V"), jAd, (jint)y);
		env->DeleteLocalRef(jAd);
	}
	
	int getX(const char *ad)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jAd = env->NewStringUTF(ad);
		int x = (int)env->CallStaticIntMethod(cls_, env->GetStaticMethodID(cls_, "getX", "(Ljava/lang/String;)I"), jAd);
		env->DeleteLocalRef(jAd);
		return x;
	}
	
	int getY(const char *ad)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jAd = env->NewStringUTF(ad);
		int y = (int)env->CallStaticIntMethod(cls_, env->GetStaticMethodID(cls_, "getY", "(Ljava/lang/String;)I"), jAd);
		env->DeleteLocalRef(jAd);
		return y;
	}
	
	int getWidth(const char *ad)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jAd = env->NewStringUTF(ad);
		int width = (int)env->CallStaticIntMethod(cls_, env->GetStaticMethodID(cls_, "getWidth", "(Ljava/lang/String;)I"), jAd);
		env->DeleteLocalRef(jAd);
		return width;
	}
	
	int getHeight(const char *ad)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jAd = env->NewStringUTF(ad);
		int height = (int)env->CallStaticIntMethod(cls_, env->GetStaticMethodID(cls_, "getHeight", "(Ljava/lang/String;)I"), jAd);
		env->DeleteLocalRef(jAd);
		return height;
	}
	
	void onAdReceived(jstring jAd, jstring jAdType)
	{
		JNIEnv *env = g_getJNIEnv();

		const char *ad = env->GetStringUTFChars(jAd, NULL);
		const char *type = env->GetStringUTFChars(jAdType, NULL);
			
		gads_SimpleEvent *event = (gads_SimpleEvent*)gevent_CreateEventStruct2(
			sizeof(gads_SimpleEvent),
			offsetof(gads_SimpleEvent, ad), ad,
			offsetof(gads_SimpleEvent, type), type);

		env->ReleaseStringUTFChars(jAdType, ad);
		env->ReleaseStringUTFChars(jAd, type);
			
		gevent_EnqueueEvent(gid_, callback_s, GADS_AD_RECEIVED_EVENT, event, 1, this);
	}
	
	void onAdFailed(jstring jAd, jstring jAdType, jstring jerror)
	{
		JNIEnv *env = g_getJNIEnv();

		const char *error = jerror ? env->GetStringUTFChars(jerror, NULL) : NULL;
		const char *ad = env->GetStringUTFChars(jAd, NULL);
		const char *type = env->GetStringUTFChars(jAdType, NULL);

		gads_AdFailedEvent *event = (gads_AdFailedEvent*)gevent_CreateEventStruct3(
			sizeof(gads_AdFailedEvent),
			offsetof(gads_AdFailedEvent, ad), ad,
			offsetof(gads_AdFailedEvent, type), type,
			offsetof(gads_AdFailedEvent, error), error);

		env->ReleaseStringUTFChars(jerror, error);
		env->ReleaseStringUTFChars(jAd, ad);
		env->ReleaseStringUTFChars(jAdType, type);

		gevent_EnqueueEvent(gid_, callback_s, GADS_AD_FAILED_EVENT, event, 1, this);
	}
	
	void onAdActionBegin(jstring jAd, jstring jAdType)
	{
		JNIEnv *env = g_getJNIEnv();
		const char *ad = env->GetStringUTFChars(jAd, NULL);
		const char *type = env->GetStringUTFChars(jAdType, NULL);
			
		gads_SimpleEvent *event = (gads_SimpleEvent*)gevent_CreateEventStruct2(
			sizeof(gads_SimpleEvent),
			offsetof(gads_SimpleEvent, ad), ad,
			offsetof(gads_SimpleEvent, type), type);

		env->ReleaseStringUTFChars(jAdType, ad);
		env->ReleaseStringUTFChars(jAd, type);
		gevent_EnqueueEvent(gid_, callback_s, GADS_AD_ACTION_BEGIN_EVENT, event, 1, this);
	}
	
	void onAdActionEnd(jstring jAd, jstring jAdType)
	{
		JNIEnv *env = g_getJNIEnv();
		const char *ad = env->GetStringUTFChars(jAd, NULL);
		const char *type = env->GetStringUTFChars(jAdType, NULL);
			
		gads_SimpleEvent *event = (gads_SimpleEvent*)gevent_CreateEventStruct2(
			sizeof(gads_SimpleEvent),
			offsetof(gads_SimpleEvent, ad), ad,
			offsetof(gads_SimpleEvent, type), type);

		env->ReleaseStringUTFChars(jAdType, ad);
		env->ReleaseStringUTFChars(jAd, type);
		gevent_EnqueueEvent(gid_, callback_s, GADS_AD_ACTION_END_EVENT, event, 1, this);
	}
	
	void onAdDismissed(jstring jAd, jstring jAdType)
	{
		JNIEnv *env = g_getJNIEnv();
		const char *ad = env->GetStringUTFChars(jAd, NULL);
		const char *type = env->GetStringUTFChars(jAdType, NULL);
			
		gads_SimpleEvent *event = (gads_SimpleEvent*)gevent_CreateEventStruct2(
			sizeof(gads_SimpleEvent),
			offsetof(gads_SimpleEvent, ad), ad,
			offsetof(gads_SimpleEvent, type), type);

		env->ReleaseStringUTFChars(jAdType, ad);
		env->ReleaseStringUTFChars(jAd, type);
		gevent_EnqueueEvent(gid_, callback_s, GADS_AD_DISMISSED_EVENT, event, 1, this);
	}
	
	void onAdDisplayed(jstring jAd, jstring jAdType)
	{
		JNIEnv *env = g_getJNIEnv();
		const char *ad = env->GetStringUTFChars(jAd, NULL);
		const char *type = env->GetStringUTFChars(jAdType, NULL);
		
		gads_SimpleEvent *event = (gads_SimpleEvent*)gevent_CreateEventStruct2(
            sizeof(gads_SimpleEvent),
            offsetof(gads_SimpleEvent, ad), ad,
            offsetof(gads_SimpleEvent, type), type);
			
		env->ReleaseStringUTFChars(jAdType, ad);
		env->ReleaseStringUTFChars(jAd, type);
		gevent_EnqueueEvent(gid_, callback_s, GADS_AD_DISPLAYED_EVENT, event, 1, this);
	}
	
	void onAdError(jstring jAd, jstring jerror)
	{
		JNIEnv *env = g_getJNIEnv();

		const char *error = jerror ? env->GetStringUTFChars(jerror, NULL) : NULL;
		const char *ad = env->GetStringUTFChars(jAd, NULL);

		gads_AdErrorEvent *event = (gads_AdErrorEvent*)gevent_CreateEventStruct2(
			sizeof(gads_AdErrorEvent),
			offsetof(gads_AdErrorEvent, ad), ad,
			offsetof(gads_AdErrorEvent, error), error);

		if (jerror)
			env->ReleaseStringUTFChars(jerror, error);
		env->ReleaseStringUTFChars(jAd, ad);
		gevent_EnqueueEvent(gid_, callback_s, GADS_AD_ERROR_EVENT, event, 1, this);
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
		((GAds*)udata)->callback(type, event);
	}

	void callback(int type, void *event)
	{
		callbackList_.dispatchEvent(type, event);
	}

private:
	gevent_CallbackList callbackList_;

private:
	jclass cls_;
	jclass clsSparse;
	g_id gid_;
};

extern "C" {

void Java_com_giderosmobile_android_plugins_ads_Ads_onAdReceived(JNIEnv *env, jclass clz, jstring jAd, jstring jAdType, jlong data)
{
	((GAds*)data)->onAdReceived(jAd, jAdType);
}

void Java_com_giderosmobile_android_plugins_ads_Ads_onAdFailed(JNIEnv *env, jclass clz, jstring jAd, jstring jAdType, jstring jerror, jlong data)
{
	((GAds*)data)->onAdFailed(jAd, jAdType, jerror);
}

void Java_com_giderosmobile_android_plugins_ads_Ads_onAdActionBegin(JNIEnv *env, jclass clz, jstring jAd, jstring jAdType, jlong data)
{
	((GAds*)data)->onAdActionBegin(jAd, jAdType);
}

void Java_com_giderosmobile_android_plugins_ads_Ads_onAdActionEnd(JNIEnv *env, jclass clz, jstring jAd, jstring jAdType, jlong data)
{
	((GAds*)data)->onAdActionEnd(jAd, jAdType);
}

void Java_com_giderosmobile_android_plugins_ads_Ads_onAdDismissed(JNIEnv *env, jclass clz, jstring jAd, jstring jAdType, jlong data)
{
	((GAds*)data)->onAdDismissed(jAd, jAdType);
}

void Java_com_giderosmobile_android_plugins_ads_Ads_onAdDisplayed(JNIEnv *env, jclass clz, jstring jAd, jstring jAdType, jlong data)
{
	((GAds*)data)->onAdDisplayed(jAd, jAdType);
}

void Java_com_giderosmobile_android_plugins_ads_Ads_onAdError(JNIEnv *env, jclass clz, jstring jAd, jstring jerror, jlong data)
{
	((GAds*)data)->onAdError(jAd, jerror);
}

}

static GAds *s_ads = NULL;

extern "C" {

int gads_isAvailable()
{
	return 1;
}

void gads_init()
{
	s_ads = new GAds;
}

void gads_cleanup()
{
	if(s_ads)
	{
		delete s_ads;
		s_ads = NULL;
	}
}

void gads_initialize(const char *ad)
{
	if(s_ads)
	{
		s_ads->init(ad);
	}
}

void gads_destroy(const char *ad)
{
	if(s_ads != NULL)
	{
		s_ads->destroy(ad);
	}
}

void gads_setKey(const char *ad, gads_Parameter *params)
{
	if(s_ads)
	{
		s_ads->setKey(ad, params);
	}
}

void gads_loadAd(const char *ad, gads_Parameter *params)
{
	if(s_ads)
	{
		s_ads->loadAd(ad, params);
	}
}

void gads_showAd(const char *ad, gads_Parameter *params)
{
	if(s_ads)
	{
		s_ads->showAd(ad, params);
	}
}

void gads_hideAd(const char *ad, const char *type)
{
	if(s_ads)
	{
		s_ads->hideAd(ad, type);
	}
}

void gads_enableTesting(const char *ad)
{
	if(s_ads)
	{
		s_ads->enableTesting(ad);
	}
}

void gads_setAlignment(const char *ad, const char *hor, const char *ver)
{
	if(s_ads)
	{
		s_ads->setAlignment(ad, hor, ver);
	}
}

void gads_setX(const char *ad, int x)
{
	if(s_ads)
	{
		s_ads->setX(ad, x);
	}
}

void gads_setY(const char *ad, int y)
{
	if(s_ads)
	{
		s_ads->setY(ad, y);
	}
}

int gads_getX(const char *ad)
{
	return s_ads->getX(ad);
}

int gads_getY(const char *ad)
{
	return s_ads->getY(ad);
}

int gads_getWidth(const char *ad)
{
	return s_ads->getWidth(ad);
}

int gads_getHeight(const char *ad)
{
	return s_ads->getHeight(ad);
}

g_id gads_addCallback(gevent_Callback callback, void *udata)
{
	return s_ads->addCallback(callback, udata);
}

void gads_removeCallback(gevent_Callback callback, void *udata)
{
	if(s_ads)
	{
		s_ads->removeCallback(callback, udata);
	}
}

void gads_removeCallbackWithGid(g_id gid)
{
	if(s_ads)
	{
		s_ads->removeCallbackWithGid(gid);
	}
}

}
