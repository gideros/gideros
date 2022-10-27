#include <vector>
#include <string>
#include <jni.h>
#include "platform.h"

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


bool setTextInput(int type,const char *buffer,int selstart,int selend,const char *label,const char *actionLabel, const char *hintText, const char *context)
{
	JNIEnv *env = g_getJNIEnv();

	jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
	jstring jbuf = env->NewStringUTF(buffer);
	jstring jlabel = env->NewStringUTF(label);
	jstring jaction = env->NewStringUTF(actionLabel);
	jstring jhint = env->NewStringUTF(hintText);
	jstring jcontext = env->NewStringUTF(context);

	if (selstart>0) {
		int n=0;
		for (int k=0;(k<selstart)&&(buffer[k]);k++)
			if ((buffer[k]&0xC0)!=0x80) n++;
		selstart=n;
	}
	if (selend>0) {
		int n=0;
		for (int k=0;(k<selend)&&(buffer[k]);k++)
			if ((buffer[k]&0xC0)!=0x80) n++;
		selend=n;
	}

	jmethodID setKeepAwakeID = env->GetStaticMethodID(localRefCls, "setTextInput", "(ILjava/lang/String;IILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z");
	jboolean ret=env->CallStaticBooleanMethod(localRefCls, setKeepAwakeID, (jint)type,jbuf,(jint)selstart,(jint)selend,jlabel,jaction,jhint,jcontext);
	env->DeleteLocalRef(jbuf);
	env->DeleteLocalRef(jlabel);
	env->DeleteLocalRef(jaction);
	env->DeleteLocalRef(jhint);
	env->DeleteLocalRef(jcontext);
	env->DeleteLocalRef(localRefCls);
	return ret;
}

int setClipboard(std::string data,std::string mimeType, int luaFunc) {
	return -1;
}

int getClipboard(std::string &data,std::string &mimeType, int luaFunc) {
	return -1;
}

int getKeyboardModifiers() {
	return 0;
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

std::string getAppId(){
    JNIEnv *env = g_getJNIEnv();

	jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
	jmethodID getMethodID = env->GetStaticMethodID(localRefCls, "getAppId", "()Ljava/lang/String;");
	jstring jresult = (jstring)env->CallStaticObjectMethod(localRefCls, getMethodID);
	const char *result = env->GetStringUTFChars(jresult, NULL);
	std::string sresult = result;
	env->ReleaseStringUTFChars(jresult, result);
	env->DeleteLocalRef(jresult);
	env->DeleteLocalRef(localRefCls);

	return sresult;
}

void getSafeDisplayArea(int &x,int &y,int &w,int &h)
{
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

std::vector<gapplication_Variant> g_getsetProperty(bool set, const char* what, std::vector<gapplication_Variant> &args)
{
	std::vector<gapplication_Variant> rets;
	gapplication_Variant r;
	if (!set) {
		JNIEnv *env = g_getJNIEnv();

		jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
		jmethodID getMethodID = env->GetStaticMethodID(localRefCls, "getProperty", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
		jstring jwhat = env->NewStringUTF(what?what:"");
		jstring jarg = env->NewStringUTF((args.size()>0)?args[0].s.c_str():"");
		jstring jresult = (jstring)env->CallStaticObjectMethod(localRefCls, getMethodID,jwhat,jarg);
		if (jresult) {
			r.type=gapplication_Variant::STRING;
			const char *result = env->GetStringUTFChars(jresult, NULL);
			r.s= result;
			rets.push_back(r);
			env->ReleaseStringUTFChars(jresult, result);
			env->DeleteLocalRef(jresult);
		}
		env->DeleteLocalRef(jwhat);
		env->DeleteLocalRef(jarg);
		env->DeleteLocalRef(localRefCls);
	}
	return rets;
}

bool gapplication_checkPermission(const char *what) {
	JNIEnv *env = g_getJNIEnv();

	jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
	jmethodID checkPermID = env->GetStaticMethodID(localRefCls, "checkPermission", "(Ljava/lang/String;)Z");
	jstring jperm = env->NewStringUTF(what);
	jboolean result = env->CallStaticBooleanMethod(localRefCls, checkPermID, jperm);
	env->DeleteLocalRef(jperm);
	env->DeleteLocalRef(localRefCls);
	return result;
}

void gapplication_requestPermissions(std::vector<std::string> perms) {
	JNIEnv *env = g_getJNIEnv();

	jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
	jmethodID checkPermID = env->GetStaticMethodID(localRefCls, "requestPermissions", "([Ljava/lang/String;)V");

	  jobjectArray jperms = env->NewObjectArray(perms.size(),env->FindClass("java/lang/String"),0);

	  for(size_t i=0;i<perms.size();i++)
	  {
	    jstring str = env->NewStringUTF(perms[i].c_str());
	    env->SetObjectArrayElement(jperms,i,str);
		env->DeleteLocalRef(str);
	  }

	env->CallStaticVoidMethod(localRefCls, checkPermID, jperms);
	env->DeleteLocalRef(jperms);
	env->DeleteLocalRef(localRefCls);
}
