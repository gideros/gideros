#include <jni.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

void openUrl(const char *url)
{
	JNIEnv *env = g_getJNIEnv();
	
	jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
	jmethodID openUrlID = env->GetStaticMethodID(localRefCls, "openUrl", "(Ljava/lang/String;)V");
	jstring jurl = env->NewStringUTF(url);
	env->CallStaticVoidMethod(localRefCls, openUrlID, jurl);
	env->DeleteLocalRef(jurl);
	env->DeleteLocalRef(localRefCls);
}

bool canOpenUrl(const char *url)
{
	JNIEnv *env = g_getJNIEnv();
	
	jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
	jmethodID canOpenUrlID = env->GetStaticMethodID(localRefCls, "canOpenUrl", "(Ljava/lang/String;)Z");
	jstring jurl = env->NewStringUTF(url);
	jboolean result = env->CallStaticBooleanMethod(localRefCls, canOpenUrlID, jurl);
	env->DeleteLocalRef(jurl);
	env->DeleteLocalRef(localRefCls);
	return result;
}
