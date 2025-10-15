#include <jni.h>
#include <stdlib.h>
#include <glog.h>
#include "agesignal.h"

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

class GGoogleAS
{
public:
	GGoogleAS()
	{
		gid_ = g_NextId();
		
		JNIEnv *env = g_getJNIEnv();

		jclass localClass = env->FindClass("com/giderosmobile/android/plugins/agesignal/AgeSignal");
		cls_ = (jclass)env->NewGlobalRef(localClass);
		env->DeleteLocalRef(localClass);

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "init", "(J)V"), (jlong)this);
	}

	~GGoogleAS()
	{
		JNIEnv *env = g_getJNIEnv();

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "cleanup", "()V"));
		
		env->DeleteGlobalRef(cls_);
		
		gevent_RemoveEventsWithGid(gid_);
	}
	
	void checkAgeSignals()
	{
		JNIEnv *env = g_getJNIEnv();

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "checkAgeSignals", "()V"));
	}
	
	
	void onAgeSignals(jstring installId, jstring status, jlong approvalDate, jint ageLower, jint ageUpper)
	{

		JNIEnv *env = g_getJNIEnv();
		const char *c_id = env->GetStringUTFChars(installId, NULL);
		const char *c_status = env->GetStringUTFChars(status, NULL);

		gagesignals_AgeSignalsEvent *event = (gagesignals_AgeSignalsEvent*)gevent_CreateEventStruct2(
			sizeof(gagesignals_AgeSignalsEvent),
			offsetof(gagesignals_AgeSignalsEvent, installId), c_id,
			offsetof(gagesignals_AgeSignalsEvent, status), c_status);
		event->approvalDate=approvalDate;
		event->ageLower=ageLower;
		event->ageUpper=ageUpper;

		env->ReleaseStringUTFChars(installId, c_id);
		env->ReleaseStringUTFChars(status, c_status);
		gevent_EnqueueEvent(gid_, callback_s, AGESIGNAL_AGE_SIGNALS_ENV, event, 1, this);
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
		((GGoogleAS*)udata)->callback(type, event);
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

void Java_com_giderosmobile_android_plugins_agesignal_AgeSignal_onAgeSignals(JNIEnv *env, jclass clz, jstring installId, jstring status, jlong approvalDate, jint ageLower, jint ageUpper, jlong data)
{
	((GGoogleAS*)data)->onAgeSignals(installId, status, approvalDate, ageLower, ageUpper);
}

}

static GGoogleAS *s_glvl = NULL;

extern "C" {

void gagesignal_init()
{
	s_glvl = new GGoogleAS;
}

void gagesignal_cleanup()
{
	delete s_glvl;
	s_glvl = NULL;
}

void gagesignal_checkAgeSignals()
{
	s_glvl->checkAgeSignals();
}

g_id gagesignal_addCallback(gevent_Callback callback, void *udata)
{
	return s_glvl->addCallback(callback, udata);
}

void gagesignal_removeCallback(gevent_Callback callback, void *udata)
{
	s_glvl->removeCallback(callback, udata);
}

void gagesignal_removeCallbackWithGid(g_id gid)
{
	s_glvl->removeCallbackWithGid(gid);
}

}
