#include <vector>
#include <string>
#include <jni.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

std::vector<std::string> getDeviceInfo()
{
	JNIEnv *env = g_getJNIEnv();

	std::vector<std::string> result;

	result.push_back("Android");

	jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
	jmethodID getAndroidVersionID = env->GetStaticMethodID(localRefCls, "getAndroidVersion", "()Ljava/lang/String;");
	jstring jstr = (jstring)env->CallStaticObjectMethod(localRefCls, getAndroidVersionID);
	const char *str = env->GetStringUTFChars(jstr, NULL);
	result.push_back(str);
	env->ReleaseStringUTFChars(jstr, str);
	env->DeleteLocalRef(jstr);
	env->DeleteLocalRef(localRefCls);

	return result;
}
