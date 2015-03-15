#include <giftiz.h>
#include <jni.h>
#include <stdlib.h>
#include <glog.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

class GFZ
{
public:
	GFZ()
	{
		gid_ = g_NextId();
		
		JNIEnv *env = g_getJNIEnv();

		jclass localClass = env->FindClass("com/giderosmobile/android/plugins/giftiz/GGiftiz");
		cls_ = (jclass)env->NewGlobalRef(localClass);
		env->DeleteLocalRef(localClass);

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "init", "(J)V"), (jlong)this);
	}

	~GFZ()
	{
		JNIEnv *env = g_getJNIEnv();

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "cleanup", "()V"));
		
		env->DeleteGlobalRef(cls_);
		
		gevent_RemoveEventsWithGid(gid_);
	}
	
	bool isAvailable()
	{
		JNIEnv *env = g_getJNIEnv();
		return (bool)env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "isAvailable", "()Z"));
	}
		
	void missionComplete()
	{
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "missionComplete", "()V"));
	}
	
	void purchaseMade(float amount)
	{
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "purchaseMade", "(F)V"), (jfloat)amount);
	}
	
	int getButtonState()
	{
		JNIEnv *env = g_getJNIEnv();
		int state = (int)env->CallStaticIntMethod(cls_, env->GetStaticMethodID(cls_, "getButtonState", "()I"));
		return state;
	}
	
	void buttonClicked()
	{
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "buttonClicked", "()V"));
	}
	
	void onButtonUpdate(jint state)
	{
		JNIEnv *env = g_getJNIEnv();

		giftiz_Button *event = (giftiz_Button*)malloc(sizeof(giftiz_Button));
		
		event->state = state;
		
		gevent_EnqueueEvent(gid_, callback_s, GIFTIZ_BUTTON_STATE_CHANGE, event, 1, this);
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
		((GFZ*)udata)->callback(type, event);
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

void Java_com_giderosmobile_android_plugins_giftiz_GGiftiz_onButtonUpdate(JNIEnv *env, jclass clz, jint state, jlong data)
{
	((GFZ*)data)->onButtonUpdate(state);
}

}

static GFZ *s_gfz = NULL;

extern "C" {

void giftiz_init()
{
	s_gfz = new GFZ;
}

void giftiz_cleanup()
{
	delete s_gfz;
	s_gfz = NULL;
}

bool giftiz_isAvailable()
{
	return s_gfz->isAvailable();
}

void giftiz_missionComplete()
{
	s_gfz->missionComplete();
}

void giftiz_purchaseMade(float amount)
{
	s_gfz->purchaseMade(amount);
}

int giftiz_getButtonState()
{
	return s_gfz->getButtonState();
}

void giftiz_buttonClicked()
{
	s_gfz->buttonClicked();
}

g_id giftiz_addCallback(gevent_Callback callback, void *udata)
{
	return s_gfz->addCallback(callback, udata);
}

void giftiz_removeCallback(gevent_Callback callback, void *udata)
{
	s_gfz->removeCallback(callback, udata);
}

void giftiz_removeCallbackWithGid(g_id gid)
{
	s_gfz->removeCallbackWithGid(gid);
}

}
