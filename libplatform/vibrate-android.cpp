#include <jni.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

void vibrate()
{
	JNIEnv *env = g_getJNIEnv();

	jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
	jmethodID vibrateID = env->GetStaticMethodID(localRefCls, "vibrate", "()V");
	env->CallStaticVoidMethod(localRefCls, vibrateID);
	env->DeleteLocalRef(localRefCls);
}
