#include "ggeolocation.h"
#include <jni.h>
#include <gevent.h>
#include <set>
#include <vector>
#include <map>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

class GGGeolocationManager
{
public:
	GGGeolocationManager()
	{
		locationStartCount_ = 0;
		headingStartCount_ = 0;

        gid_ = g_NextId();
	}

	virtual ~GGGeolocationManager()
	{
		if (locationStartCount_ > 0)
			stopUpdatingLocationHelper();
		if (headingStartCount_ > 0)
			stopUpdatingHeadingHelper();

		gevent_RemoveEventsWithGid(gid_);
	}

	bool isAvailable()
	{
		JNIEnv *env = g_getJNIEnv();

		jclass cls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
		jboolean result = env->CallStaticBooleanMethod(cls, env->GetStaticMethodID(cls, "isGeolocationAvailable_s", "()Z"));
		env->DeleteLocalRef(cls);
		
		return result;
	}

	bool isHeadingAvailable()
	{
		JNIEnv *env = g_getJNIEnv();

		jclass cls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
		jboolean result = env->CallStaticBooleanMethod(cls, env->GetStaticMethodID(cls, "isHeadingAvailable_s", "()Z"));
		env->DeleteLocalRef(cls);
		
		return result;
	}

	void startUpdatingLocation()
	{
		locationStartCount_++;
		if (locationStartCount_ == 1)
			startUpdatingLocationHelper();
	}
	
	void stopUpdatingLocation()
	{
		if (locationStartCount_ > 0)
		{
			locationStartCount_--;
			if (locationStartCount_ == 0)
				stopUpdatingLocationHelper();
		}
	}

	void startUpdatingHeading()
	{
		headingStartCount_++;
		if (headingStartCount_ == 1)
			startUpdatingHeadingHelper();
	}

	
	void stopUpdatingHeading()
	{
		if (headingStartCount_ > 0)
		{
			headingStartCount_--;
			if (headingStartCount_ == 0)
				stopUpdatingHeadingHelper();
		}
	}	

private:
	void startUpdatingLocationHelper()
	{
		JNIEnv *env = g_getJNIEnv();

		jclass cls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
		env->CallStaticVoidMethod(cls, env->GetStaticMethodID(cls, "startUpdatingLocation_s", "()V"));
		env->DeleteLocalRef(cls);
	}
	
	void stopUpdatingLocationHelper()
	{
		JNIEnv *env = g_getJNIEnv();

		jclass cls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
		env->CallStaticVoidMethod(cls, env->GetStaticMethodID(cls, "stopUpdatingLocation_s", "()V"));
		env->DeleteLocalRef(cls);
	}

	void startUpdatingHeadingHelper()
	{
		JNIEnv *env = g_getJNIEnv();

		jclass cls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
		env->CallStaticVoidMethod(cls, env->GetStaticMethodID(cls, "startUpdatingHeading_s", "()V"));
		env->DeleteLocalRef(cls);
	}
	
	void stopUpdatingHeadingHelper()
	{
		JNIEnv *env = g_getJNIEnv();

		jclass cls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
		env->CallStaticVoidMethod(cls, env->GetStaticMethodID(cls, "stopUpdatingHeading_s", "()V"));
		env->DeleteLocalRef(cls);
	}	
	
private:
	int locationStartCount_;
	int headingStartCount_;

public:
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
        static_cast<GGGeolocationManager*>(udata)->callbackList_.dispatchEvent(type, event);
    }
    
    void enqueueEvent(int type, void *event, int free)
    {
        gevent_EnqueueEvent(gid_, callback_s, type, event, free, this);
    }
    
private:
    gevent_CallbackList callbackList_;
    g_id gid_;
};

static GGGeolocationManager *s_manager = NULL;
static bool s_acceptEvents = false;

extern "C" {

void Java_com_giderosmobile_android_player_Geolocation_onLocationChanged(JNIEnv *env, jclass clz, jdouble latitude, jdouble longitude, jdouble altitude)
{
	if (!s_acceptEvents)
		return;

	ggeolocation_LocationUpdateEvent *event = (ggeolocation_LocationUpdateEvent*)malloc(sizeof(ggeolocation_LocationUpdateEvent));
	event->latitude = latitude;
	event->longitude = longitude;
	event->altitude = altitude;

	s_manager->enqueueEvent(GGEOLOCATION_LOCATION_UPDATE_EVENT, event, 1);
}

void Java_com_giderosmobile_android_player_Geolocation_onHeadingChanged(JNIEnv *env, jclass clz, jdouble magneticHeading, jdouble trueHeading)
{
	if (!s_acceptEvents)
		return;

	ggeolocation_HeadingUpdateEvent *event = (ggeolocation_HeadingUpdateEvent*)malloc(sizeof(ggeolocation_HeadingUpdateEvent));
	event->magneticHeading = magneticHeading;
	event->trueHeading = trueHeading;

	s_manager->enqueueEvent(GGEOLOCATION_HEADING_UPDATE_EVENT, event, 1);
}

}

extern "C" {

void ggeolocation_init()
{
    s_manager = new GGGeolocationManager;
	s_acceptEvents = true;
}

void ggeolocation_cleanup()
{
	s_acceptEvents = false;
	delete s_manager;
	s_manager = NULL;	
}

int ggeolocation_isAvailable()
{
	return s_manager->isAvailable();	
}
int ggeolocation_isHeadingAvailable()
{
	return s_manager->isHeadingAvailable();
}

void ggeolocation_setAccuracy(double accuracy)
{

}

double ggeolocation_getAccuracy()
{
    return 0;
}

void ggeolocation_setThreshold(double threshold)
{

}

double ggeolocation_getThreshold()
{
    return 0;
}

void ggeolocation_startUpdatingLocation()
{
	s_manager->startUpdatingLocation();
}

void ggeolocation_stopUpdatingLocation()
{
	s_manager->stopUpdatingLocation();
}

void ggeolocation_startUpdatingHeading()
{
	s_manager->startUpdatingHeading();
}

void ggeolocation_stopUpdatingHeading()
{
	s_manager->stopUpdatingHeading();
}

g_id ggeolocation_addCallback(gevent_Callback callback, void *udata)
{
    return s_manager->addCallback(callback, udata);
}

void ggeolocation_removeCallback(gevent_Callback callback, void *udata)
{
    s_manager->removeCallback(callback, udata);
}

void ggeolocation_removeCallbackWithGid(g_id gid)
{
    s_manager->removeCallbackWithGid(gid);    
}

}
