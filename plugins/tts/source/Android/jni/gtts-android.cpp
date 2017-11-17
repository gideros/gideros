#include <jni.h>

#include <map>

#include <math.h>
#include <stdlib.h>
#include <android/log.h>
#include "../../../../tts/source/Common/gtts.h"

#define  LOG_TAG    "GTTS"

#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

static std::map<int,void*> tts_udata;
static jclass cls_;
static gevent_Callback callback_;

static char *GetJStringDup(JNIEnv *AEnv, jstring AStr) {
  if (!AStr) {
    return strdup("");
  }

  const char *s = AEnv->GetStringUTFChars(AStr,NULL);
  char *sd=strdup(s?s:"");
  AEnv->ReleaseStringUTFChars(AStr,s);
  return sd;
}


static void GetJStringContent(JNIEnv *AEnv, jstring AStr, std::string &ARes) {
  if (!AStr) {
    ARes.clear();
    return;
  }

  const char *s = AEnv->GetStringUTFChars(AStr,NULL);
  ARes=s;
  AEnv->ReleaseStringUTFChars(AStr,s);
}

static void GetJStringCStr(JNIEnv *AEnv, jstring AStr, char *buf, int max) {
  if (!AStr) {
	*buf=0;
    return;
  }

  const char *s = AEnv->GetStringUTFChars(AStr,NULL);
  strncpy(buf,s,max);
  buf[max-1]=0;
  AEnv->ReleaseStringUTFChars(AStr,s);
}

extern "C" {

static g_id gid = g_NextId();

JNIEXPORT void JNICALL Java_com_giderosmobile_android_plugins_tts_TTSManager_eventTtsInit(JNIEnv *env, jclass clz, jint tid)
{
	if (tts_udata.find(tid)!=tts_udata.end())
		gevent_EnqueueEvent(gid, callback_, GTts::GTTS_INIT_COMPLETE_EVENT, NULL, 1, tts_udata[tid]);
}

JNIEXPORT void JNICALL Java_com_giderosmobile_android_plugins_tts_TTSManager_eventTtsError(JNIEnv *env, jclass clz, jint tid, jstring error)
{
	if (tts_udata.find(tid)!=tts_udata.end())
	{
		void *event=GetJStringDup(env,error);
		gevent_EnqueueEvent(gid, callback_, GTts::GTTS_ERROR_EVENT, event, 1, tts_udata[tid]);
	}
}

JNIEXPORT void JNICALL Java_com_giderosmobile_android_plugins_tts_TTSManager_eventTtsUtterance(JNIEnv *env, jclass clz, jint tid, jstring state, jstring utterance)
{
	if (tts_udata.find(tid)!=tts_udata.end()) {
		char *cstate=GetJStringDup(env,state);
		char *cutterance=GetJStringDup(env,utterance);
		char *event=(char *)malloc(strlen(cstate)+strlen(cutterance)+2);
		strcpy(event,cstate);
		strcpy(event+strlen(cstate)+1,cutterance);
		free(cstate);
		free(cutterance);
		gevent_EnqueueEvent(gid, callback_, GTts::GTTS_UTTERANCE_COMPLETE_EVENT, event, 1, tts_udata[tid]);
	}
}


}

//extern "C" {


void JNI_OnUnload(JavaVM *vm, void *reserved) {
	if (cls_)
	{
		JNIEnv *env = g_getJNIEnv();
		env->DeleteGlobalRef(cls_);
		cls_=NULL;
	}
}


void gtts_Init(gevent_Callback callback)
{
	JNIEnv *env = g_getJNIEnv();
	callback_=callback;

	if (!cls_) {
		jclass localClass = env->FindClass("com/giderosmobile/android/plugins/tts/TTSManager");
		if (localClass)
		{
			cls_ = (jclass)env->NewGlobalRef(localClass);
			env->DeleteLocalRef(localClass);
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "Init", "()V"));
		}
	}
}

void gtts_Cleanup()
{
	JNIEnv *env = g_getJNIEnv();
	env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "Cleanup", "()V"));
	tts_udata.clear();
	gevent_RemoveEventsWithGid(gid);
}

class AndroidTts : public GTts {
	int tid;
public:
	AndroidTts(lua_State *L,const char *lang,float speed,float pitch);
	~AndroidTts();
	bool SetLanguage(const char *lang);
	bool SetPitch(float pitch);
	bool SetSpeed(float speed);
	void Stop();
	void Shutdown();
	bool Speak(const char *text,const char *utteranceId);
    float GetSpeed();
    float GetPitch();
    float GetVolume();
    bool SetVolume(float v);
    bool SetVoice(const char *v);
};


AndroidTts::AndroidTts(lua_State *L,const char *lang,float speed,float pitch) : GTts(L)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jlang=env->NewStringUTF(lang);
		tid=env->CallStaticIntMethod(cls_, env->GetStaticMethodID(cls_, "Create", "(Ljava/lang/String;FF)I"),jlang,speed,pitch);
		tts_udata[tid]=this;
	}
AndroidTts::~AndroidTts() {
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "Destroy", "(I)V"),tid);
		tts_udata.erase(tid);
	}
		bool AndroidTts::SetLanguage(const char *lang)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jlang=env->NewStringUTF(lang);
		return env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "SetLanguage", "(ILjava/lang/String;)Z"),tid,jlang);
	}

	bool AndroidTts::SetPitch(float pitch)
	{
		JNIEnv *env = g_getJNIEnv();
		return env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "SetPitch", "(IF)Z"),tid,pitch);
	}

	bool AndroidTts::SetSpeed(float speed)
	{
		JNIEnv *env = g_getJNIEnv();
		return env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "SetSpeechRate", "(IF)Z"),tid,speed);
	}

	void AndroidTts::Stop()
	{
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "Stop", "(I)V"),tid);
	}

	void AndroidTts::Shutdown()
	{
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "Shutdown", "(I)V"),tid);
	}

	bool AndroidTts::Speak(const char *text,const char *utteranceId)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jtext=env->NewStringUTF(text);
		jstring juid=env->NewStringUTF(utteranceId);
		return env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "Speak", "(ILjava/lang/String;Ljava/lang/String;)Z"),tid,jtext,juid);
	}

    float AndroidTts::GetSpeed() { return 0; } //TODO
    float AndroidTts::GetPitch() { return 1; } //TODO
    float AndroidTts::GetVolume() { return 1; } //TODO
    bool AndroidTts::SetVolume(float v) { return false; } //TODO
    bool AndroidTts::SetVoice(const char *v) { return false; } //TODO

GTts *gtts_Create(lua_State *L,const char *lang,float speed,float pitch)
{
	return new AndroidTts(L,lang,speed,pitch);
}

std::vector<struct VoiceInfo> gtts_GetVoicesInstalled(){
	std::vector<struct VoiceInfo> voices;
    return voices;
}

