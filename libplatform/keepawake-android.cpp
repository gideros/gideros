#include <jni.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

void setKeepAwake(bool awake)
{
	JNIEnv *env = g_getJNIEnv();

	jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
	jmethodID setKeepAwakeID = env->GetStaticMethodID(localRefCls, "setKeepAwake", "(Z)V");
	env->CallStaticVoidMethod(localRefCls, setKeepAwakeID, (jboolean)awake);
	env->DeleteLocalRef(localRefCls);
}
