#include <jni.h>
#include <glog.h>
#include "sharebinder.h"

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

class GSD
{
public:
	GSD()
	{		
		JNIEnv *env = g_getJNIEnv();

		jclass localClass = env->FindClass("com/giderosmobile/android/plugins/gshare/Share");
		cls_ = (jclass)env->NewGlobalRef(localClass);
		env->DeleteLocalRef(localClass);
	}

	~GSD()
	{
		JNIEnv *env = g_getJNIEnv();
		env->DeleteGlobalRef(cls_);
	}

	
	bool share(std::map<std::string,std::string> values)
	{
		JNIEnv *env = g_getJNIEnv();
		  jclass mapClass = env->FindClass("java/util/HashMap");
		  if(mapClass == NULL)
			return false;

		  jmethodID init = env->GetMethodID(mapClass, "<init>", "()V");
		  jobject hashMap = env->NewObject(mapClass, init);
		  jmethodID put = env->GetMethodID(mapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

		  std::map<std::string, std::string>::const_iterator citr = values.begin();
		  for( ; citr != values.end(); ++citr) {
			jstring jstr = env->NewStringUTF(citr->first.c_str());

			jbyteArray jdata = NULL;
			size_t datasize = citr->second.size();
			if (datasize > 0)
			{
				jdata = env->NewByteArray(datasize);
				env->SetByteArrayRegion(jdata, 0, datasize, (jbyte*)citr->second.c_str());
			}

		    env->CallObjectMethod(hashMap, put, jstr, jdata);

			if (jdata)
				env->DeleteLocalRef(jdata);
			env->DeleteLocalRef(jstr);
		  }

		jboolean result = env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "share", "(Ljava/lang/Map;)Z"),hashMap);
		  env->DeleteLocalRef(hashMap);
		  env->DeleteLocalRef(mapClass);

		return result;
	}
private:
	jclass cls_;
};

static GSD *s_sd = NULL;

bool gshare_Share(std::map<std::string,std::string> values)
{
	if(s_sd != NULL)
	{
		return s_sd->share(values);
	}
	return false;
}

void gshare_Init()
{
	s_sd = new GSD;
}
void gshare_Cleanup()
{
	if(s_sd)
	{
		delete s_sd;
		s_sd = NULL;
	}
}

