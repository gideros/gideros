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

	{
		jstring jstr = (jstring)env->CallStaticObjectMethod(localRefCls, env->GetStaticMethodID(localRefCls, "getVersion", "()Ljava/lang/String;"));
		const char *str = env->GetStringUTFChars(jstr, NULL);
		result.push_back(str);
		env->ReleaseStringUTFChars(jstr, str);
		env->DeleteLocalRef(jstr);
	}
	{
		jstring jstr = (jstring)env->CallStaticObjectMethod(localRefCls, env->GetStaticMethodID(localRefCls, "getManufacturer", "()Ljava/lang/String;"));
		const char *str = env->GetStringUTFChars(jstr, NULL);
		result.push_back(str);
		env->ReleaseStringUTFChars(jstr, str);
		env->DeleteLocalRef(jstr);
	}
	{
		jstring jstr = (jstring)env->CallStaticObjectMethod(localRefCls, env->GetStaticMethodID(localRefCls, "getModel", "()Ljava/lang/String;"));
		const char *str = env->GetStringUTFChars(jstr, NULL);
		result.push_back(str);
		env->ReleaseStringUTFChars(jstr, str);
		env->DeleteLocalRef(jstr);
	}
    {
		jstring jstr = (jstring)env->CallStaticObjectMethod(localRefCls, env->GetStaticMethodID(localRefCls, "getDeviceType", "()Ljava/lang/String;"));
		const char *str = env->GetStringUTFChars(jstr, NULL);
		result.push_back(str);
		env->ReleaseStringUTFChars(jstr, str);
		env->DeleteLocalRef(jstr);
	}

	env->DeleteLocalRef(localRefCls);

	return result;
}

void setKeepAwake(bool awake)
{
	JNIEnv *env = g_getJNIEnv();

	jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
	jmethodID setKeepAwakeID = env->GetStaticMethodID(localRefCls, "setKeepAwake", "(Z)V");
	env->CallStaticVoidMethod(localRefCls, setKeepAwakeID, (jboolean)awake);
	env->DeleteLocalRef(localRefCls);
}

bool setKeyboardVisibility(bool visible)
{
	JNIEnv *env = g_getJNIEnv();

	jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
	jmethodID setKeepAwakeID = env->GetStaticMethodID(localRefCls, "setKeyboardVisibility", "(Z)Z");
	jboolean ret=env->CallStaticBooleanMethod(localRefCls, setKeepAwakeID, (jboolean)visible);
	env->DeleteLocalRef(localRefCls);
	return ret;
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

void vibrate(int ms)
{
	JNIEnv *env = g_getJNIEnv();

	jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
	jmethodID vibrateID = env->GetStaticMethodID(localRefCls, "vibrate", "(I)V");
	env->CallStaticVoidMethod(localRefCls, vibrateID, (jint)ms);
	env->DeleteLocalRef(localRefCls);
}

void setWindowSize(int width, int height){

}

void setFullScreen(bool fullScreen){

}

std::string getDeviceName(){
    JNIEnv *env = g_getJNIEnv();

	jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
	jmethodID getMethodID = env->GetStaticMethodID(localRefCls, "getDeviceName", "()Ljava/lang/String;");
	jstring jresult = (jstring)env->CallStaticObjectMethod(localRefCls, getMethodID);
	const char *result = env->GetStringUTFChars(jresult, NULL);
	std::string sresult = result;
	env->ReleaseStringUTFChars(jresult, result);
	env->DeleteLocalRef(jresult);
	env->DeleteLocalRef(localRefCls);

	return sresult;
}

void g_exit()
{
	JNIEnv *env = g_getJNIEnv();

	jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
	jmethodID finishActivityID = env->GetStaticMethodID(localRefCls, "finishActivity", "()V");
	env->CallStaticVoidMethod(localRefCls, finishActivityID);
	env->DeleteLocalRef(localRefCls);
}

static int s_fps = 60;

extern "C" {

int g_getFps()
{
    return s_fps;
}

void g_setFps(int fps)
{
    s_fps = fps;

	JNIEnv *env = g_getJNIEnv();
	jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
	jmethodID setFpsID = env->GetStaticMethodID(localRefCls, "setFps", "(I)V");
	env->CallStaticVoidMethod(localRefCls, setFpsID, (jint)fps);
	env->DeleteLocalRef(localRefCls);
}

}

bool g_checkStringProperty(bool isSet, const char* what){
    return false;
}
void g_setProperty(const char* what, const char* arg){

}

const char* g_getProperty(const char* what, const char* arg){
	return "";
}
