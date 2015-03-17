#include <storedetector.h>
#include <jni.h>
#include <glog.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

static std::string getString(JNIEnv *env, jstring jstr)
{
	const char *str = env->GetStringUTFChars(jstr, NULL);
	std::string result = str;
	env->ReleaseStringUTFChars(jstr, str);
	return result;
}

class GSD
{
public:
	GSD()
	{		
		JNIEnv *env = g_getJNIEnv();

		jclass localClass = env->FindClass("com/giderosmobile/android/plugins/store/StoreDetector");
		cls_ = (jclass)env->NewGlobalRef(localClass);
		env->DeleteLocalRef(localClass);
	}

	~GSD()
	{
		JNIEnv *env = g_getJNIEnv();
		env->DeleteGlobalRef(cls_);
	}
	
	int isConsole()
	{
		JNIEnv *env = g_getJNIEnv();
		return env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "isConsole", "()Z"));
	}
	
	std::string getStore()
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jstr = (jstring)env->CallStaticObjectMethod(cls_, env->GetStaticMethodID(cls_, "getStore", "()Ljava/lang/String;"));
		std::string ret = getString(env, jstr);
		env->DeleteLocalRef(jstr);
		return ret;
	}
private:
	jclass cls_;
};

static GSD *s_sd = NULL;

extern "C" {

void sd_init()
{
	s_sd = new GSD;
}

void sd_cleanup()
{
	if(s_sd)
	{
		delete s_sd;
		s_sd = NULL;
	}
}

int sd_isConsole()
{
	if(s_sd)
	{
		return s_sd->isConsole();
	}
	return 0;
}

std::string sd_getStore()
{
	if(s_sd != NULL)
	{
		return s_sd->getStore();
	}
	return "android";
}
}
