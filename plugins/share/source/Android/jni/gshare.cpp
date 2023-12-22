#include <jni.h>
#include <glog.h>
#include "sharebinder.h"

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

static gevent_CallbackList callbackList_;
static void callback_s(int type, void *event, void *udata)
{
  callbackList_.dispatchEvent(type, event);
}

extern "C" {

static g_id gid = g_NextId();

JNIEXPORT void JNICALL Java_com_giderosmobile_android_plugins_gshare_Share_onImportResult(JNIEnv *env, jclass clz, jint status, jstring name, jstring mime, jbyteArray jdata)
{
	const char *sname = name?env->GetStringUTFChars(name,NULL):NULL;
	const char *smime = mime?env->GetStringUTFChars(mime,NULL):NULL;

	size_t sdataSize=0;
	jbyte *sdata=NULL;
	if (jdata) {
		sdataSize=env->GetArrayLength(jdata);
		sdata = env->GetByteArrayElements(jdata, 0);
	}

	gfileshare_ResultEvent *event = (gfileshare_ResultEvent*)gevent_CreateEventStruct2(sizeof(gfileshare_ResultEvent)+sdataSize,
			offsetof(gfileshare_ResultEvent,name),sname,
			offsetof(gfileshare_ResultEvent,mime),smime);
	memcpy((char *)(event+1),sdata,sdataSize);
	event->dataSize=sdataSize;
	event->status=status;

	gevent_EnqueueEvent(gid, callback_s, GFILESHARE_IMPORT_RESULT_EVENT, event, 1, NULL);
	if (sdata)
		env->ReleaseByteArrayElements(jdata,sdata,0);
	if (sname)
		env->ReleaseStringUTFChars(name,sname);
	if (smime)
		env->ReleaseStringUTFChars(mime,smime);
}

JNIEXPORT void JNICALL Java_com_giderosmobile_android_plugins_gshare_Share_onExportResult(JNIEnv *env, jclass clz, jint status)
{
	gfileshare_ResultEvent *event = (gfileshare_ResultEvent*)gevent_CreateEventStruct2(sizeof(gfileshare_ResultEvent),
			offsetof(gfileshare_ResultEvent,name),NULL,
			offsetof(gfileshare_ResultEvent,mime),NULL);
	event->status=status;
	event->dataSize=0;

	gevent_EnqueueEvent(gid, callback_s, GFILESHARE_EXPORT_RESULT_EVENT, event, 1, NULL);
}

}

static jclass cls_;

bool gshare_Share(std::map<std::string,std::string> values)
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

	jboolean result = env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "share", "(Ljava/util/Map;)Z"),hashMap);
	  env->DeleteLocalRef(hashMap);
	  env->DeleteLocalRef(mapClass);

	return result;
}

void gshare_Init()
{
	JNIEnv *env = g_getJNIEnv();

	jclass localClass = env->FindClass("com/giderosmobile/android/plugins/gshare/Share");
	cls_ = (jclass)env->NewGlobalRef(localClass);
	env->DeleteLocalRef(localClass);
	env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "Init", "()V"));
}

void gshare_Cleanup()
{
	if (cls_) {
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "Cleanup", "()V"));
		env->DeleteGlobalRef(cls_);
	}
	cls_=NULL;
}

bool gshare_Import(const char *mime, const char *extension)
{
	JNIEnv *env = g_getJNIEnv();
	jstring jmime = mime?env->NewStringUTF(mime):NULL;
	jstring jextension = extension?env->NewStringUTF(extension):NULL;
	jboolean res= env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "Import", "(Ljava/lang/String;Ljava/lang/String;)Z"),jmime,jextension);
	if (jextension)
		env->DeleteLocalRef(jextension);
	if (jmime)
		env->DeleteLocalRef(jmime);
	return res;
}

bool gshare_Export(const char *data,size_t dataSize,const char *mime, const char *filename)
{
	JNIEnv *env = g_getJNIEnv();
	jbyteArray jdata = env->NewByteArray(dataSize);
	if (!jdata) return false;
    env->SetByteArrayRegion (jdata, 0, dataSize, (jbyte *)data);
	jstring jmime = mime?env->NewStringUTF(mime):NULL;
	jstring jfilename = filename?env->NewStringUTF(filename):NULL;
	jboolean res= env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "Export", "([BLjava/lang/String;Ljava/lang/String;)Z"),jdata,jmime,jfilename);
	env->DeleteLocalRef(jdata);
	if (jfilename)
		env->DeleteLocalRef(jfilename);
	if (jmime)
		env->DeleteLocalRef(jmime);
	return res;
}

int gshare_Capabilities()
{
	return CAP_SHARE|CAP_IMPORT|CAP_EXPORT;
}

g_id gshare_AddCallback(gevent_Callback callback, void *udata)
{
    return callbackList_.addCallback(callback, udata);
}

void gshare_RemoveCallback(gevent_Callback callback, void *udata)
{
    callbackList_.removeCallback(callback, udata);
}

