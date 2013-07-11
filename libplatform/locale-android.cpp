#include <string>
#include <jni.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

std::string getLocale()
{
	JNIEnv *env = g_getJNIEnv();

	jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
	jmethodID getLocaleID = env->GetStaticMethodID(localRefCls, "getLocale", "()Ljava/lang/String;");
	jstring jresult = (jstring)env->CallStaticObjectMethod(localRefCls, getLocaleID);
	const char *result = env->GetStringUTFChars(jresult, NULL);
	std::string sresult = result;
	env->ReleaseStringUTFChars(jresult, result);
	env->DeleteLocalRef(jresult);
	env->DeleteLocalRef(localRefCls);

	return sresult;
}


std::string getLanguage()
{
	JNIEnv *env = g_getJNIEnv();

	jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
	jmethodID getLanguageID = env->GetStaticMethodID(localRefCls, "getLanguage", "()Ljava/lang/String;");
	jstring jresult = (jstring)env->CallStaticObjectMethod(localRefCls, getLanguageID);
	const char *result = env->GetStringUTFChars(jresult, NULL);
	std::string sresult = result;
	env->ReleaseStringUTFChars(jresult, result);
	env->DeleteLocalRef(jresult);
	env->DeleteLocalRef(localRefCls);

	return sresult;
}
