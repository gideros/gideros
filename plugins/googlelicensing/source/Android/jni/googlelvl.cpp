#include <googlelvl.h>
#include <jni.h>
#include <stdlib.h>
#include <glog.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

class GGoogleLVL
{
public:
	GGoogleLVL()
	{
		gid_ = g_NextId();
		
		JNIEnv *env = g_getJNIEnv();

		jclass localClass = env->FindClass("com/giderosmobile/android/plugins/googlelicensing/GoogleLVL");
		cls_ = (jclass)env->NewGlobalRef(localClass);
		env->DeleteLocalRef(localClass);

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "init", "(J)V"), (jlong)this);
	}

	~GGoogleLVL()
	{
		JNIEnv *env = g_getJNIEnv();

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "cleanup", "()V"));
		
		env->DeleteGlobalRef(cls_);
		
		gevent_RemoveEventsWithGid(gid_);
	}
	
	void setKey(const char *key)
	{
		JNIEnv *env = g_getJNIEnv();
		
		jstring jKey = env->NewStringUTF(key);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setKey", "(Ljava/lang/String;)V"), jKey);
		env->DeleteLocalRef(jKey);
	}
	
	void checkLicense()
	{
		JNIEnv *env = g_getJNIEnv();

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "checkLicense", "()V"));
	}
	
	void checkExpansion()
	{
		JNIEnv *env = g_getJNIEnv();

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "checkExpansion", "()V"));
	}
	
	void cellularDownload(int use)
	{
		JNIEnv *env = g_getJNIEnv();

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "cellularDownload", "(I)V"), (jint)use);
	}
	
	void onAllow()
	{
		gevent_EnqueueEvent(gid_, callback_s, GOOGLELVL_ALLOW_EVENT, NULL, 1, this);
	}
	
	void onDisallow()
	{
		gevent_EnqueueEvent(gid_, callback_s, GOOGLELVL_DISALLOW_EVENT, NULL, 1, this);
	}
	
	void onRetry()
	{
		gevent_EnqueueEvent(gid_, callback_s, GOOGLELVL_RETRY_EVENT, NULL, 1, this);
	}
	
	void onDownloadRequired()
	{
		gevent_EnqueueEvent(gid_, callback_s, GOOGLELVL_DOWNLOAD_REQUIRED_EVENT, NULL, 1, this);
	}
	
	void onDownloadNotRequired()
	{
		gevent_EnqueueEvent(gid_, callback_s, GOOGLELVL_DOWNLOAD_NOT_REQUIRED_EVENT, NULL, 1, this);
	}
	
	void onDownloadProgress(jfloat speed, jlong time, jlong progress, jlong total)
	{
		JNIEnv *env = g_getJNIEnv();
		
		ggooglelvl_ProgressEvent *event = (ggooglelvl_ProgressEvent*)malloc(sizeof(ggooglelvl_ProgressEvent));
		event->speed = (float)speed;
		event->time = (long)time;
		event->progress = (long)progress;
		event->total = (long)total;
		
		gevent_EnqueueEvent(gid_, callback_s, GOOGLELVL_DOWNLOAD_PROGRESS_EVENT, event, 1, this);
	}
	
	void onDownloadState(jstring jstate, jstring jmessage)
	{
		JNIEnv *env = g_getJNIEnv();
		const char *state = env->GetStringUTFChars(jstate, NULL);
		const char *message = env->GetStringUTFChars(jmessage, NULL);
		
		ggooglelvl_StateEvent *event = (ggooglelvl_StateEvent*)gevent_CreateEventStruct2(
			sizeof(ggooglelvl_StateEvent),
			offsetof(ggooglelvl_StateEvent, state), state,
			offsetof(ggooglelvl_StateEvent, message), message);
	
		env->ReleaseStringUTFChars(jstate, state);
		env->ReleaseStringUTFChars(jmessage, message);
		
		gevent_EnqueueEvent(gid_, callback_s, GOOGLELVL_DOWNLOAD_STATE_EVENT, event, 1, this);
	}
	
	void onError(jstring jerror)
	{
		JNIEnv *env = g_getJNIEnv();
		const char *error = env->GetStringUTFChars(jerror, NULL);
		
		ggooglelvl_SimpleEvent *event = (ggooglelvl_SimpleEvent*)gevent_CreateEventStruct1(
			sizeof(ggooglelvl_SimpleEvent),
			offsetof(ggooglelvl_SimpleEvent, error), error);
	
		env->ReleaseStringUTFChars(jerror, error);
		
		gevent_EnqueueEvent(gid_, callback_s, GOOGLELVL_ERROR_EVENT, event, 1, this);
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
		((GGoogleLVL*)udata)->callback(type, event);
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

void Java_com_giderosmobile_android_plugins_googlelicensing_GoogleLVL_onLicenseAllowed(JNIEnv *env, jclass clz, jlong data)
{
	((GGoogleLVL*)data)->onAllow();
}

void Java_com_giderosmobile_android_plugins_googlelicensing_GoogleLVL_onLicenseDisallowed(JNIEnv *env, jclass clz, jlong data)
{
	((GGoogleLVL*)data)->onDisallow();
}

void Java_com_giderosmobile_android_plugins_googlelicensing_GoogleLVL_onLicenseRetry(JNIEnv *env, jclass clz, jlong data)
{
	((GGoogleLVL*)data)->onRetry();
}

void Java_com_giderosmobile_android_plugins_googlelicensing_GoogleLVL_onDownloadRequired(JNIEnv *env, jclass clz, jlong data)
{
	((GGoogleLVL*)data)->onDownloadRequired();
}

void Java_com_giderosmobile_android_plugins_googlelicensing_GoogleLVL_onDownloadNotRequired(JNIEnv *env, jclass clz, jlong data)
{
	((GGoogleLVL*)data)->onDownloadNotRequired();
}

void Java_com_giderosmobile_android_plugins_googlelicensing_GoogleLVL_onDownloadProgress(JNIEnv *env, jclass clz, jfloat speed, jlong time, jlong progress, jlong total, jlong data)
{
	((GGoogleLVL*)data)->onDownloadProgress(speed, time, progress, total);
}

void Java_com_giderosmobile_android_plugins_googlelicensing_GoogleLVL_onDownloadState(JNIEnv *env, jclass clz, jstring jstate, jstring jmessage, jlong data)
{
	((GGoogleLVL*)data)->onDownloadState(jstate, jmessage);
}

void Java_com_giderosmobile_android_plugins_googlelicensing_GoogleLVL_onError(JNIEnv *env, jclass clz, jstring error, jlong data)
{
	((GGoogleLVL*)data)->onError(error);
}

}

static GGoogleLVL *s_glvl = NULL;

extern "C" {

void ggooglelvl_init()
{
	s_glvl = new GGoogleLVL;
}

void ggooglelvl_cleanup()
{
	delete s_glvl;
	s_glvl = NULL;
}

void ggooglelvl_setKey(const char *key)
{
	s_glvl->setKey(key);
}

void ggooglelvl_checkLicense()
{
	s_glvl->checkLicense();
}

void ggooglelvl_checkExpansion()
{
	s_glvl->checkExpansion();
}

void ggooglelvl_cellularDownload(int use){
	s_glvl->cellularDownload(use);
}

g_id ggooglelvl_addCallback(gevent_Callback callback, void *udata)
{
	return s_glvl->addCallback(callback, udata);
}

void ggooglelvl_removeCallback(gevent_Callback callback, void *udata)
{
	s_glvl->removeCallback(callback, udata);
}

void ggooglelvl_removeCallbackWithGid(g_id gid)
{
	s_glvl->removeCallbackWithGid(gid);
}

}
