#include <media.h>
#include <jni.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

class GMEDIA
{
public:
	GMEDIA()
	{
		gid_ = g_NextId();
		
		JNIEnv *env = g_getJNIEnv();

		jclass localClass = env->FindClass("com/giderosmobile/android/plugins/media/GMedia");
		cls_ = (jclass)env->NewGlobalRef(localClass);
		env->DeleteLocalRef(localClass);

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "init", "(J)V"), (jlong)this);
	}

	~GMEDIA()
	{
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "cleanup", "()V"));
		
		env->DeleteGlobalRef(cls_);
		
		gevent_RemoveEventsWithGid(gid_);
	}
	
	bool isCameraAvailable()
	{
		JNIEnv *env = g_getJNIEnv();
		return (bool)env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "isCameraAvailable", "()Z"));
	}
	
	void takePicture()
	{
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "takePicture", "()V"));
	}
	
	void takeScreenshot()
	{
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "takeScreenshot", "()V"));
	}
	
	void getPicture()
	{
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "getPicture", "()V"));
	}
	
	void savePicture(const char* path)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jpath = env->NewStringUTF(path);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "savePicture", "(Ljava/lang/String;)V"), jpath);
		env->DeleteLocalRef(jpath);
	}
	
	void playVideo(const char* path, bool force)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jpath = env->NewStringUTF(path);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "playVideo", "(Ljava/lang/String;Z)V"), jpath, (jboolean)force);
		env->DeleteLocalRef(jpath);
	}
	
	void onMediaReceived(jstring jpath)
	{
		JNIEnv *env = g_getJNIEnv();

		const char *path = env->GetStringUTFChars(jpath, NULL);

		gmedia_ReceivedEvent *event = (gmedia_ReceivedEvent*)gevent_CreateEventStruct1(
			sizeof(gmedia_ReceivedEvent),
			offsetof(gmedia_ReceivedEvent, path), path);

		env->ReleaseStringUTFChars(jpath, path);
		gevent_EnqueueEvent(gid_, callback_s, GMEDIA_RECEIVED_EVENT, event, 1, this);
	}
	
	void onMediaCanceled()
	{
		JNIEnv *env = g_getJNIEnv();
		gevent_EnqueueEvent(gid_, callback_s, GMEDIA_CANCELED_EVENT, NULL, 1, this);
	}
	
	void onMediaCompleted()
	{
		JNIEnv *env = g_getJNIEnv();
		gevent_EnqueueEvent(gid_, callback_s, GMEDIA_COMPLETED_EVENT, NULL, 1, this);
	}
	
	g_id addCallback(gevent_Callback callback, void *udata)
	{
		return callbackList_.addCallback(callback, udata);
	}
	void removeCallback(gevent_Callback callback, void *udata)
	{
		callbackList_.removeCallback(callback, udata);
	}
	void removeCallbackWithGid(g_id gid)
	{
		callbackList_.removeCallbackWithGid(gid);
	}

private:
	static void callback_s(int type, void *event, void *udata)
	{
		((GMEDIA*)udata)->callback(type, event);
	}

	void callback(int type, void *event)
	{
		callbackList_.dispatchEvent(type, event);
	}

private:
	gevent_CallbackList callbackList_;

private:
	jclass cls_;
	g_id gid_;
};

extern "C" {

void Java_com_giderosmobile_android_plugins_media_GMedia_onMediaReceived(JNIEnv *env, jclass clz, jstring path, jlong data)
{
	((GMEDIA*)data)->onMediaReceived(path);
}

void Java_com_giderosmobile_android_plugins_media_GMedia_onMediaCanceled(JNIEnv *env, jclass clz, jlong data)
{
	((GMEDIA*)data)->onMediaCanceled();
}

void Java_com_giderosmobile_android_plugins_media_GMedia_onMediaCompleted(JNIEnv *env, jclass clz, jlong data)
{
	((GMEDIA*)data)->onMediaCompleted();
}

}

static GMEDIA *s_gmedia = NULL;

extern "C" {

void gmedia_init()
{
	s_gmedia = new GMEDIA;
}

void gmedia_cleanup()
{
	delete s_gmedia;
	s_gmedia = NULL;
}

int gmedia_isCameraAvailable()
{
	return s_gmedia->isCameraAvailable();
}

void gmedia_takePicture(){
	s_gmedia->takePicture();
}

void gmedia_takeScreenshot(){
	s_gmedia->takeScreenshot();
}

void gmedia_getPicture(){
	s_gmedia->getPicture();
}

void gmedia_savePicture(const char* path){
	s_gmedia->savePicture(path);
}

void gmedia_playVideo(const char* path, int force){
	s_gmedia->playVideo(path, force);
}

g_id gmedia_addCallback(gevent_Callback callback, void *udata)
{
	return s_gmedia->addCallback(callback, udata);
}

void gmedia_removeCallback(gevent_Callback callback, void *udata)
{
	s_gmedia->removeCallback(callback, udata);
}

void gmedia_removeCallbackWithGid(g_id gid)
{
	s_gmedia->removeCallbackWithGid(gid);
}

}
