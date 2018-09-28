#include <jni.h>
#include <glog.h>
#include "megacoolbinder.h"

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

static jclass cls_;

int isConsole()
	{
	}
	
	std::string getStore()
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jstr = (jstring)env->CallStaticObjectMethod(cls_, env->GetStaticMethodID(cls_, "getStore", "()Ljava/lang/String;"));
		std::string ret = getString(env, jstr);
		env->DeleteLocalRef(jstr);
		return ret;
	}

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
	bool ret=env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "share", "(Ljava/lang/String;)Z",jfallback));
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

