#include "luaapplication.h"

namespace oculus {
void onCreate(JNIEnv *env,jobject activity,LuaApplication *app);
void onStart();
void onResume();
void onPause();
void onStop();
void onDestroy(JNIEnv *env);

void runThread();
void postCreate();

/*
================================================================================

Surface lifecycle

================================================================================
*/

void onSurfaceCreated(JNIEnv *env,jobject surface);
void onSurfaceChanged(JNIEnv *env,jobject surface);
void onSurfaceDestroyed();
void onLuaReinit();

typedef struct Input_ {
	//Generic
	int deviceId;
	int deviceType;
	int batteryPercent;
	int recenterCount;
	//Remote
	int caps;
	int buttons;
	float stickX,stickY;
    float indexTrigger;
    float gripTrigger;
} Input;
//Implemented in gideros.cpp
void doInputEvent(Input &input);
void doTick(double elapsed);
void doRender(float *vmat,float *pmat,int width, int height,bool room,bool screen);
}
