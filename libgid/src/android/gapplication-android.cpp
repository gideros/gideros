#include <gapplication.h>
#include <gapplication-android.h>
#include <jni.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

class GGApplicationManager
{
    jclass javaCls_;
public:
    GGApplicationManager()
    {
        gid_ = g_NextId();
		JNIEnv *env = g_getJNIEnv();
		jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
		javaCls_ = (jclass)env->NewGlobalRef(localRefCls);
		env->DeleteLocalRef(localRefCls);
    }
    
    ~GGApplicationManager()
    {
        gevent_RemoveEventsWithGid(gid_);
    }
    
	int getScreenDensity()
	{
		JNIEnv *env = g_getJNIEnv();
		jmethodID getScreenDensityID = env->GetStaticMethodID(javaCls_, "getScreenDensity", "()I");
		jint result = env->CallStaticIntMethod(javaCls_, getScreenDensityID);
		return result;
	}

	void requestDeviceOrientation(gapplication_Orientation iO,gapplication_AutoOrientation iAutoRot) {
		JNIEnv *env = g_getJNIEnv();
		jmethodID requestDeviceOrientationID = env->GetStaticMethodID(javaCls_, "requestDeviceOrientation", "(II)V");
		env->CallStaticVoidMethod(javaCls_, requestDeviceOrientationID,iO,iAutoRot);
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
    
    static void callback_s(int type, void *event, void *udata)
    {
        static_cast<GGApplicationManager*>(udata)->callbackList_.dispatchEvent(type, event);
    }
    
    void enqueueEvent(int type, void *event, int free)
    {
        gevent_EnqueueEvent(gid_, callback_s, type, event, free, this);
    }
    
private:
    gevent_CallbackList callbackList_;
    g_id gid_;
};


static GGApplicationManager *s_manager = NULL;

extern "C" {

void gapplication_init()
{
    s_manager = new GGApplicationManager;
}

void gapplication_cleanup()
{
    delete s_manager;
    s_manager = NULL;
}

g_id gapplication_addCallback(gevent_Callback callback, void *udata)
{
    return s_manager->addCallback(callback, udata);
}

void gapplication_removeCallback(gevent_Callback callback, void *udata)
{
    s_manager->removeCallback(callback, udata);
}

void gapplication_removeCallbackWithGid(g_id gid)
{
    s_manager->removeCallbackWithGid(gid);
}

void gapplication_exit()
{
        
}

int gapplication_getScreenDensity(int *ldpi)
{
	return s_manager->getScreenDensity();
}

void gapplication_requestDeviceOrientation(gapplication_Orientation iO,gapplication_AutoOrientation iAutoRot) {
	s_manager->requestDeviceOrientation(iO,iAutoRot);
}

void gapplication_enqueueEvent(int type, void *event, int free)
{
    s_manager->enqueueEvent(type, event, free);
}

}
