#include <jni.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

void g_exit()
{
	JNIEnv *env = g_getJNIEnv();

	jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
	jmethodID finishActivityID = env->GetStaticMethodID(localRefCls, "finishActivity", "()V");
	env->CallStaticVoidMethod(localRefCls, finishActivityID);
	env->DeleteLocalRef(localRefCls);
}