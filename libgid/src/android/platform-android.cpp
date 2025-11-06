#include <vector>
#include <string>
#include <jni.h>
#include "platform.h"

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

static     jclass javaCls_=0;
static jclass JCLS() {
	if (javaCls_) return javaCls_;
	JNIEnv *env = g_getJNIEnv();
	jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
	javaCls_ = (jclass)env->NewGlobalRef(localRefCls);
	env->DeleteLocalRef(localRefCls);
	return javaCls_;
}

std::vector<std::string> getDeviceInfo()
{
	JNIEnv *env = g_getJNIEnv();

	std::vector<std::string> result;

	result.push_back("Android");

	{
		jstring jstr = (jstring)env->CallStaticObjectMethod(JCLS(), env->GetStaticMethodID(JCLS(), "getVersion", "()Ljava/lang/String;"));
		const char *str = env->GetStringUTFChars(jstr, NULL);
		result.push_back(str);
		env->ReleaseStringUTFChars(jstr, str);
		env->DeleteLocalRef(jstr);
	}
	{
		jstring jstr = (jstring)env->CallStaticObjectMethod(JCLS(), env->GetStaticMethodID(JCLS(), "getManufacturer", "()Ljava/lang/String;"));
		const char *str = env->GetStringUTFChars(jstr, NULL);
		result.push_back(str);
		env->ReleaseStringUTFChars(jstr, str);
		env->DeleteLocalRef(jstr);
	}
	{
		jstring jstr = (jstring)env->CallStaticObjectMethod(JCLS(), env->GetStaticMethodID(JCLS(), "getModel", "()Ljava/lang/String;"));
		const char *str = env->GetStringUTFChars(jstr, NULL);
		result.push_back(str);
		env->ReleaseStringUTFChars(jstr, str);
		env->DeleteLocalRef(jstr);
	}
    {
		jstring jstr = (jstring)env->CallStaticObjectMethod(JCLS(), env->GetStaticMethodID(JCLS(), "getDeviceType", "()Ljava/lang/String;"));
		const char *str = env->GetStringUTFChars(jstr, NULL);
		result.push_back(str);
		env->ReleaseStringUTFChars(jstr, str);
		env->DeleteLocalRef(jstr);
	}

	return result;
}

void setKeepAwake(bool awake)
{
	JNIEnv *env = g_getJNIEnv();

	jmethodID setKeepAwakeID = env->GetStaticMethodID(JCLS(), "setKeepAwake", "(Z)V");
	env->CallStaticVoidMethod(JCLS(), setKeepAwakeID, (jboolean)awake);
}

bool setKeyboardVisibility(bool visible)
{
	JNIEnv *env = g_getJNIEnv();

	jmethodID setKeepAwakeID = env->GetStaticMethodID(JCLS(), "setKeyboardVisibility", "(Z)Z");
	jboolean ret=env->CallStaticBooleanMethod(JCLS(), setKeepAwakeID, (jboolean)visible);
	return ret;
}


bool setTextInput(int type,const char *buffer,int selstart,int selend,const char *label,const char *actionLabel, const char *hintText, const char *context)
{
	JNIEnv *env = g_getJNIEnv();

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

	jmethodID setKeepAwakeID = env->GetStaticMethodID(JCLS(), "setTextInput", "(ILjava/lang/String;IILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z");
	jboolean ret=env->CallStaticBooleanMethod(JCLS(), setKeepAwakeID, (jint)type,jbuf,(jint)selstart,(jint)selend,jlabel,jaction,jhint,jcontext);
	env->DeleteLocalRef(jbuf);
	env->DeleteLocalRef(jlabel);
	env->DeleteLocalRef(jaction);
	env->DeleteLocalRef(jhint);
	env->DeleteLocalRef(jcontext);
	return ret;
}

int setClipboard(std::string data,std::string mimeType, int luaFunc) {
	JNIEnv *env = g_getJNIEnv();

	jstring jdata = env->NewStringUTF(data.c_str());
	jstring jmime = env->NewStringUTF(mimeType.c_str());
	jmethodID setClipboardID = env->GetStaticMethodID(JCLS(), "setClipboard", "(Ljava/lang/String;Ljava/lang/String;)Z");
	jboolean ret=env->CallStaticBooleanMethod(JCLS(), setClipboardID, jdata, jmime);
	env->DeleteLocalRef(jmime);
	env->DeleteLocalRef(jdata);
	return ret?1:-1;
}

int getClipboard(std::string &data,std::string &mimeType, int luaFunc) {
	JNIEnv *env = g_getJNIEnv();

	jstring jmime = env->NewStringUTF(mimeType.c_str());
	jmethodID getClipboardID = env->GetStaticMethodID(JCLS(), "getClipboard", "(Ljava/lang/String;)[Ljava/lang/String;");
	jobjectArray sArray =(jobjectArray)env->CallStaticObjectMethod(JCLS(), getClipboardID, jmime);
	if (sArray) {
		jstring jodata = (jstring)env->GetObjectArrayElement(sArray, 0);
		jstring jomime = (jstring)env->GetObjectArrayElement(sArray, 1);
		const char *sodata=env->GetStringUTFChars(jodata, NULL);
		data=sodata;
		env->ReleaseStringUTFChars(jodata, sodata);
		env->DeleteLocalRef(jodata);
		const char *somime=env->GetStringUTFChars(jomime, NULL);
		mimeType=somime;
		env->ReleaseStringUTFChars(jomime, somime);
		env->DeleteLocalRef(jomime);

		env->DeleteLocalRef(sArray);
	}

	env->DeleteLocalRef(jmime);
	return sArray?1:-1;
}

int getKeyboardModifiers() {
	return 0;
}

std::string getLocale()
{
	JNIEnv *env = g_getJNIEnv();

	jmethodID getLocaleID = env->GetStaticMethodID(JCLS(), "getLocale", "()Ljava/lang/String;");
	jstring jresult = (jstring)env->CallStaticObjectMethod(JCLS(), getLocaleID);
	const char *result = env->GetStringUTFChars(jresult, NULL);
	std::string sresult = result;
	env->ReleaseStringUTFChars(jresult, result);
	env->DeleteLocalRef(jresult);

	return sresult;
}



void openUrl(const char *url)
{
	JNIEnv *env = g_getJNIEnv();
	
	jmethodID openUrlID = env->GetStaticMethodID(JCLS(), "openUrl", "(Ljava/lang/String;)V");
	jstring jurl = env->NewStringUTF(url);
	env->CallStaticVoidMethod(JCLS(), openUrlID, jurl);
	env->DeleteLocalRef(jurl);
}

bool canOpenUrl(const char *url)
{
	JNIEnv *env = g_getJNIEnv();
	
	jmethodID canOpenUrlID = env->GetStaticMethodID(JCLS(), "canOpenUrl", "(Ljava/lang/String;)Z");
	jstring jurl = env->NewStringUTF(url);
	jboolean result = env->CallStaticBooleanMethod(JCLS(), canOpenUrlID, jurl);
	env->DeleteLocalRef(jurl);
	return result;
}


std::string getLanguage()
{
	JNIEnv *env = g_getJNIEnv();

	jmethodID getLanguageID = env->GetStaticMethodID(JCLS(), "getLanguage", "()Ljava/lang/String;");
	jstring jresult = (jstring)env->CallStaticObjectMethod(JCLS(), getLanguageID);
	const char *result = env->GetStringUTFChars(jresult, NULL);
	std::string sresult = result;
	env->ReleaseStringUTFChars(jresult, result);
	env->DeleteLocalRef(jresult);

	return sresult;
}

std::string getTimezone()
{
	JNIEnv *env = g_getJNIEnv();

	jmethodID getTimezoneID = env->GetStaticMethodID(JCLS(), "getTimezone", "()Ljava/lang/String;");
	jstring jresult = (jstring)env->CallStaticObjectMethod(JCLS(), getTimezoneID);
	const char *result = env->GetStringUTFChars(jresult, NULL);
	std::string sresult = result;
	env->ReleaseStringUTFChars(jresult, result);
	env->DeleteLocalRef(jresult);

	return sresult;
}

void vibrate(int ms)
{
	JNIEnv *env = g_getJNIEnv();

	jmethodID vibrateID = env->GetStaticMethodID(JCLS(), "vibrate", "(I)V");
	env->CallStaticVoidMethod(JCLS(), vibrateID, (jint)ms);
}

void setWindowSize(int width, int height){

}

void setFullScreen(bool fullScreen){

}

std::string getDeviceName(){
    JNIEnv *env = g_getJNIEnv();

	jmethodID getMethodID = env->GetStaticMethodID(JCLS(), "getDeviceName", "()Ljava/lang/String;");
	jstring jresult = (jstring)env->CallStaticObjectMethod(JCLS(), getMethodID);
	const char *result = env->GetStringUTFChars(jresult, NULL);
	std::string sresult = result;
	env->ReleaseStringUTFChars(jresult, result);
	env->DeleteLocalRef(jresult);

	return sresult;
}

std::string getAppId(){
    JNIEnv *env = g_getJNIEnv();

	jmethodID getMethodID = env->GetStaticMethodID(JCLS(), "getAppId", "()Ljava/lang/String;");
	jstring jresult = (jstring)env->CallStaticObjectMethod(JCLS(), getMethodID);
	const char *result = env->GetStringUTFChars(jresult, NULL);
	std::string sresult = result;
	env->ReleaseStringUTFChars(jresult, result);
	env->DeleteLocalRef(jresult);

	return sresult;
}

void getSafeDisplayArea(int &x,int &y,int &w,int &h)
{
	JNIEnv *env = g_getJNIEnv();

	jmethodID methodID = env->GetStaticMethodID(JCLS(), "getSafeArea", "()[I");
	jintArray iarr=(jintArray) env->CallStaticObjectMethod(JCLS(), methodID);
    int *p= env->GetIntArrayElements(iarr, NULL);
    x=p[0];
    y=p[2];
    w=p[1];
    h=p[3];
    env->ReleaseIntArrayElements(iarr,p, JNI_ABORT);
}

void g_exit()
{
	JNIEnv *env = g_getJNIEnv();

	jmethodID finishActivityID = env->GetStaticMethodID(JCLS(), "finishActivity", "()V");
	env->CallStaticVoidMethod(JCLS(), finishActivityID);
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
	jmethodID setFpsID = env->GetStaticMethodID(JCLS(), "setFps", "(I)V");
	env->CallStaticVoidMethod(JCLS(), setFpsID, (jint)fps);
}

}

std::vector<gapplication_Variant> g_getsetProperty(bool set, const char* what, std::vector<gapplication_Variant> &args)
{
	std::vector<gapplication_Variant> rets;
	gapplication_Variant r;
	if (!set) {
		JNIEnv *env = g_getJNIEnv();

		jmethodID getMethodID = env->GetStaticMethodID(JCLS(), "getProperty", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
		jstring jwhat = env->NewStringUTF(what?what:"");
		jstring jarg = env->NewStringUTF((args.size()>0)?args[0].s.c_str():"");
		jstring jresult = (jstring)env->CallStaticObjectMethod(JCLS(), getMethodID,jwhat,jarg);
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
	}
	else {
		JNIEnv *env = g_getJNIEnv();

		jmethodID getMethodID = env->GetStaticMethodID(JCLS(), "setProperty", "(Ljava/lang/String;Ljava/lang/String;)V");
		jstring jwhat = env->NewStringUTF(what?what:"");
		jstring jarg = env->NewStringUTF((args.size()>0)?args[0].s.c_str():"");
		env->CallStaticVoidMethod(JCLS(), getMethodID,jwhat,jarg);
		env->DeleteLocalRef(jwhat);
		env->DeleteLocalRef(jarg);
	}
	return rets;
}

bool gapplication_checkPermission(const char *what) {
	JNIEnv *env = g_getJNIEnv();

	jmethodID checkPermID = env->GetStaticMethodID(JCLS(), "checkPermission", "(Ljava/lang/String;)Z");
	jstring jperm = env->NewStringUTF(what);
	jboolean result = env->CallStaticBooleanMethod(JCLS(), checkPermID, jperm);
	env->DeleteLocalRef(jperm);
	return result;
}

void gapplication_requestPermissions(std::vector<std::string> perms) {
	JNIEnv *env = g_getJNIEnv();

	jmethodID checkPermID = env->GetStaticMethodID(JCLS(), "requestPermissions", "([Ljava/lang/String;)V");

	  jobjectArray jperms = env->NewObjectArray(perms.size(),env->FindClass("java/lang/String"),0);

	  for(size_t i=0;i<perms.size();i++)
	  {
	    jstring str = env->NewStringUTF(perms[i].c_str());
	    env->SetObjectArrayElement(jperms,i,str);
		env->DeleteLocalRef(str);
	  }

	env->CallStaticVoidMethod(JCLS(), checkPermID, jperms);
	env->DeleteLocalRef(jperms);
}
