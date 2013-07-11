#include <jni.h>
#include <javanativebridge.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

static int s_fps = 60;

extern "C" {

int g_getFps()
{
    return s_fps;
}

void g_setFps(int fps)
{
    s_fps = fps;

	JNIEnv *env = g_getJNIEnv();
	jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
	jmethodID setFpsID = env->GetStaticMethodID(localRefCls, "setFps", "(I)V");
	env->CallStaticVoidMethod(localRefCls, setFpsID, (jint)fps);
	env->DeleteLocalRef(localRefCls);
}

}
