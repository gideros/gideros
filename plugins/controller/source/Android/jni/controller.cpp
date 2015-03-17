#include <controller.h>
#include <jni.h>
#include <stdlib.h>
#include <glog.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

static const char* getString(JNIEnv *env, jstring jstr)
{
	const char *str = env->GetStringUTFChars(jstr, NULL);
	std::string result = str;
	env->ReleaseStringUTFChars(jstr, str);
	return result.c_str();
}

class GHID
{
public:
	GHID()
	{
		gid_ = g_NextId();
		
		JNIEnv *env = g_getJNIEnv();

		jclass localClass = env->FindClass("com/giderosmobile/android/plugins/controller/GControllerManager");
		cls_ = (jclass)env->NewGlobalRef(localClass);
		env->DeleteLocalRef(localClass);
		
		jclass class_sparse = env->FindClass("android/util/SparseArray");
		clsSparse = static_cast<jclass>(env->NewGlobalRef(class_sparse));
		env->DeleteLocalRef(class_sparse);

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "init", "(J)V"), (jlong)this);
	}

	~GHID()
	{
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "cleanup", "()V"));
		
		env->DeleteGlobalRef(cls_);
		env->DeleteGlobalRef(clsSparse);
		
		gevent_RemoveEventsWithGid(gid_);
	}
	
	int isAnyAvailable()
	{
		JNIEnv *env = g_getJNIEnv();
		return (int)env->CallStaticIntMethod(cls_, env->GetStaticMethodID(cls_, "isAnyAvailable", "()I"));
	}
	
	int getPlayerCount()
	{
		JNIEnv *env = g_getJNIEnv();
		return (int)env->CallStaticIntMethod(cls_, env->GetStaticMethodID(cls_, "getPlayerCount", "()I"));
	}
	
	const char* getControllerName(int player)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jstr = (jstring)env->CallStaticObjectMethod(cls_, env->GetStaticMethodID(cls_, "getControllerName", "(I)Ljava/lang/String;"), (jint)player);
		const char* ret = getString(env, jstr);
		env->DeleteLocalRef(jstr);
		return ret;
	}
	
	void vibrate(int player, long ms)
	{
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "vibrate", "(IJ)V"), (jint)player, (jlong)ms);
	}
	
	int* getPlayers(int* size)
	{
		JNIEnv *env = g_getJNIEnv();
		jintArray ptr = (jintArray) env->CallStaticObjectMethod(cls_, env->GetStaticMethodID(cls_, "getPlayers", "()[I"));
		jsize len = env->GetArrayLength(ptr);

		if(len == 0)
		{
			return NULL;
		}
		*size = len;
		int* ret = (int*)env->GetIntArrayElements(ptr, 0);
	
		env->DeleteLocalRef(ptr);
		return &ret[0];
		//return NULL;
	}
	
	void onKeyDownEvent(jint keyCode, jint playerId)
	{
		JNIEnv *env = g_getJNIEnv();

		ghid_KeyEvent *event = (ghid_KeyEvent*)malloc(sizeof(ghid_KeyEvent));
		event->keyCode = (int)keyCode;
		event->playerId = (int)playerId;
		gevent_EnqueueEvent(gid_, callback_s, GHID_KEY_DOWN_EVENT, event, 1, this);
	}
	
	void onKeyUpEvent(jint keyCode, jint playerId)
	{
		JNIEnv *env = g_getJNIEnv();

		ghid_KeyEvent *event = (ghid_KeyEvent*)malloc(sizeof(ghid_KeyEvent));
		event->keyCode = (int)keyCode;
		event->playerId = (int)playerId;
		gevent_EnqueueEvent(gid_, callback_s, GHID_KEY_UP_EVENT, event, 1, this);
	}
	
	void onRightJoystick(jfloat x, jfloat y, jdouble angle, jdouble strength, jint playerId)
	{
		JNIEnv *env = g_getJNIEnv();

		ghid_JoystickEvent *event = (ghid_JoystickEvent*)malloc(sizeof(ghid_JoystickEvent));
		event->x = (float)x;
		event->y = (float)y;
		event->angle = (double)angle;
		event->playerId = (int)playerId;
		event->strength = (double)strength;
		
		gevent_EnqueueEvent(gid_, callback_s, GHID_RIGHT_JOYSTICK_EVENT, event, 1, this);
	}
	
	void onLeftJoystick(jfloat x, jfloat y, jdouble angle, jdouble strength, jint playerId)
	{
		JNIEnv *env = g_getJNIEnv();

		ghid_JoystickEvent *event = (ghid_JoystickEvent*)malloc(sizeof(ghid_JoystickEvent));
		event->x = (float)x;
		event->y = (float)y;
		event->angle = angle;
		event->playerId = (int)playerId;
		event->strength = (double)strength;
		
		gevent_EnqueueEvent(gid_, callback_s, GHID_LEFT_JOYSTICK_EVENT, event, 1, this);
	}
	
	void onRightTrigger(jdouble strength, jint playerId)
	{
		JNIEnv *env = g_getJNIEnv();

		ghid_TriggerEvent *event = (ghid_TriggerEvent*)malloc(sizeof(ghid_TriggerEvent));
		event->strength = (double)strength;
		event->playerId = (int)playerId;
		
		gevent_EnqueueEvent(gid_, callback_s, GHID_RIGHT_TRIGGER_EVENT, event, 1, this);
	}
	
	void onLeftTrigger(jdouble strength, jint playerId)
	{
		JNIEnv *env = g_getJNIEnv();

		ghid_TriggerEvent *event = (ghid_TriggerEvent*)malloc(sizeof(ghid_TriggerEvent));
		event->strength = (double)strength;
		event->playerId = (int)playerId;
		
		gevent_EnqueueEvent(gid_, callback_s, GHID_LEFT_TRIGGER_EVENT, event, 1, this);
	}
	
	void onConnected(jint playerId)
	{
		JNIEnv *env = g_getJNIEnv();

		ghid_DeviceEvent *event = (ghid_DeviceEvent*)malloc(sizeof(ghid_DeviceEvent));
		event->playerId = (int)playerId;
		
		gevent_EnqueueEvent(gid_, callback_s, GHID_CONNECTED_EVENT, event, 1, this);
	}
	
	void onDisconnected(jint playerId)
	{
		JNIEnv *env = g_getJNIEnv();

		ghid_DeviceEvent *event = (ghid_DeviceEvent*)malloc(sizeof(ghid_DeviceEvent));
		event->playerId = (int)playerId;
		
		gevent_EnqueueEvent(gid_, callback_s, GHID_DISCONNECTED_EVENT, event, 1, this);
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
		((GHID*)udata)->callback(type, event);
	}

	void callback(int type, void *event)
	{
		callbackList_.dispatchEvent(type, event);
	}

private:
	gevent_CallbackList callbackList_;

private:
	jclass cls_;
	jclass clsSparse;
	g_id gid_;
};

extern "C" {

void Java_com_giderosmobile_android_plugins_controller_GControllerManager_onKeyDownEvent(JNIEnv *env, jclass clz, jint keyCode, jint playerId, jlong data)
{
	((GHID*)data)->onKeyDownEvent(keyCode, playerId);
}

void Java_com_giderosmobile_android_plugins_controller_GControllerManager_onKeyUpEvent(JNIEnv *env, jclass clz, jint keyCode, jint playerId, jlong data)
{
	((GHID*)data)->onKeyUpEvent(keyCode, playerId);
}

void Java_com_giderosmobile_android_plugins_controller_GControllerManager_onRightJoystick(JNIEnv *env, jclass clz, jfloat x, jfloat y, jdouble angle, jdouble strength, jint playerId, jlong data)
{
	((GHID*)data)->onRightJoystick(x, y, angle, strength, playerId);
}

void Java_com_giderosmobile_android_plugins_controller_GControllerManager_onLeftJoystick(JNIEnv *env, jclass clz, jfloat x, jfloat y, jdouble angle, jdouble strength, jint playerId, jlong data)
{
	((GHID*)data)->onLeftJoystick(x, y, angle, strength, playerId);
}

void Java_com_giderosmobile_android_plugins_controller_GControllerManager_onRightTrigger(JNIEnv *env, jclass clz, jdouble strength, jint playerId, jlong data)
{
	((GHID*)data)->onRightTrigger(strength, playerId);
}

void Java_com_giderosmobile_android_plugins_controller_GControllerManager_onLeftTrigger(JNIEnv *env, jclass clz, jdouble strength, jint playerId, jlong data)
{
	((GHID*)data)->onLeftTrigger(strength, playerId);
}

void Java_com_giderosmobile_android_plugins_controller_GControllerManager_onConnected(JNIEnv *env, jclass clz, jint playerId, jlong data)
{
	((GHID*)data)->onConnected(playerId);
}

void Java_com_giderosmobile_android_plugins_controller_GControllerManager_onDisconnected(JNIEnv *env, jclass clz, jint playerId, jlong data)
{
	((GHID*)data)->onDisconnected(playerId);
}

}

static GHID *s_ghid = NULL;

extern "C" {

void ghid_init()
{
	s_ghid = new GHID;
}

void ghid_cleanup()
{
	delete s_ghid;
	s_ghid = NULL;
}

int ghid_isAnyAvailable()
{
	return s_ghid->isAnyAvailable();
}

int ghid_getPlayerCount()
{
	return s_ghid->getPlayerCount();
}

const char* ghid_getControllerName(int player)
{
	return s_ghid->getControllerName(player);
}

void ghid_vibrate(int player, long ms)
{
	s_ghid->vibrate(player, ms);
}

int* ghid_getPlayers(int* size)
{
	return s_ghid->getPlayers(size);
}

g_id ghid_addCallback(gevent_Callback callback, void *udata)
{
	return s_ghid->addCallback(callback, udata);
}

void ghid_removeCallback(gevent_Callback callback, void *udata)
{
	s_ghid->removeCallback(callback, udata);
}

void ghid_removeCallbackWithGid(g_id gid)
{
	s_ghid->removeCallbackWithGid(gid);
}

}
