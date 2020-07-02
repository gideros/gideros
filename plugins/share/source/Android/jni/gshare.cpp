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

	
	bool share(const char *mimeType,const void *data,size_t datasize)
	{
		JNIEnv *env = g_getJNIEnv();

		jstring jstr = env->NewStringUTF(mimeType);

		jbyteArray jdata = NULL;
		if (datasize > 0)
		{
			jdata = env->NewByteArray(datasize);
			env->SetByteArrayRegion(jdata, 0, datasize, (jbyte*)data);
		}

		jboolean result = env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "share", "(Ljava/lang/String;[B)Z"),jstr,jdata);
		if (jdata)
			env->DeleteLocalRef(jdata);
		env->DeleteLocalRef(jstr);

		return result;
	}
private:
	jclass cls_;
};

static GSD *s_sd = NULL;

bool gshare_Share(const char *mimeType,const void *data,size_t datasize)
{
	if(s_sd != NULL)
	{
		return s_sd->share(mimeType,data,datasize);
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

