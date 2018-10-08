#include <jni.h>
#include <glog.h>
#include "megacoolbinder.h"
#define GIDEROS_VERSION_ONLY
#include "gideros.h"

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

static jclass cls_;

void gmegacool_Init()
{
	JNIEnv *env = g_getJNIEnv();

	jclass localClass = env->FindClass("com/giderosmobile/android/plugins/megacool/GMegaCool");
	cls_ = (jclass)env->NewGlobalRef(localClass);
	env->DeleteLocalRef(localClass);
}

void gmegacool_Destroy()
{
	JNIEnv *env = g_getJNIEnv();
	env->DeleteGlobalRef(cls_);
}

bool gmegacool_Share(const char *fallback)
{
	JNIEnv *env = g_getJNIEnv();
	jstring jfallback = env->NewStringUTF(fallback);
	bool ret=env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "share", "(Ljava/lang/String;)Z"),jfallback);
	env->DeleteLocalRef(jfallback);
	return ret;
}

bool gmegacool_StartRecording()
{
	JNIEnv *env = g_getJNIEnv();
	return env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "startRecording", "()Z"));
}

void gmegacool_StopRecording()
{
	JNIEnv *env = g_getJNIEnv();
	env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "stopRecording", "()V"));
}

bool gmegacool_SetSharingText(const char *sharingText)
{
	JNIEnv *env = g_getJNIEnv();
	jstring jsharingText = env->NewStringUTF(sharingText);
	bool ret=env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "setSharingText", "(Ljava/lang/String;)Z"),jsharingText);
	env->DeleteLocalRef(jsharingText);
	return ret;
}

extern "C" {

void Java_com_giderosmobile_android_plugins_megacool_GMegaCool_onEvent(JNIEnv *env, jclass clz, jint event)
{
	gmegacool_Event(event);
}

jstring Java_com_giderosmobile_android_plugins_megacool_GMegaCool_getGiderosVersion(JNIEnv *env, jclass clz)
{
	return env->NewStringUTF(GIDEROS_VERSION); // C style string to Java String
}

}


