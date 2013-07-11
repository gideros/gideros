#include "javanativebridge.h"
#include <stdlib.h>
#include <stdio.h>
#include <pystring.h>

static JavaVM* vm_ = NULL;
void jnb_setJavaVM(JavaVM* vm)
{
	vm_ = vm;
}

extern "C" {

JavaVM *g_getJavaVM()
{
	return vm_;
}

JNIEnv *g_getJNIEnv()
{
	JNIEnv* env = NULL;
	vm_->GetEnv((void**) &env, JNI_VERSION_1_6);
	return env;
}
}

std::vector<std::string> jnb_getLocalIPs()
{
	std::vector<std::string> result;

	JNIEnv* env = g_getJNIEnv();
	jclass clazz = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
	jmethodID javamethod = env->GetStaticMethodID(clazz, "getLocalIPs", "()Ljava/lang/String;");
	jstring jstr = (jstring)env->CallStaticObjectMethod(clazz, javamethod);

    jboolean isCopy;
    const char* str = env->GetStringUTFChars(jstr, &isCopy);
	pystring::split(str, result, "|");
    env->ReleaseStringUTFChars(jstr, str);

	return result;
}
