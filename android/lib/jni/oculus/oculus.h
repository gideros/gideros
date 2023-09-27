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

typedef struct Vector_ {
	float x,y,z;
} Vector;
typedef struct Vector4_ {
	float x,y,z,w;
} Vector4;
typedef struct Input_ {
	//Generic
	int deviceId;
	int deviceType;
	int batteryPercent;
	int recenterCount;
	//Pose
	int poseStatus;
	Vector pos;
	Vector4 rot;
	Vector velPos;
	Vector velRot;
	Vector accPos;
	Vector accRot;
	int caps;
	//Remote
	int buttons;
	float stickX,stickY;
    float indexTrigger;
    float gripTrigger;
    int trackpadStatus;
    float trackpadX;
    float trackpadY;
    int touches;
    //Hand
    Vector4 handBone[24];
    float handScale;
} Input;
//Implemented in gideros.cpp
void doInputEvent(Input &input);
void doTick(double elapsed);
void doRender(int delta,float *vmat,float *pmat,int width, int height,bool room,bool screen,bool floor);
}
