/************************************************************************************

Filename    :   VrCubeWorld_SurfaceView.c
Content     :   This sample uses a plain Android SurfaceView and handles all
                Activity and Surface life cycle events in native code. This sample
                does not use the application framework.
                This sample only uses the VrApi.
Created     :   March, 2015
Authors     :   J.M.P. van Waveren

Copyright   :   Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

*************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h> // for prctl( PR_SET_NAME )
#include <android/log.h>
#include <android/native_window_jni.h> // for native window JNI
#include <android/input.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include "luaapplication.h"
#include "oculus.h"
#include "debugging.h"

#include "ext/PassthroughFB.h"
#include "ext/HandTrackingFB.h"
#include "ext/SceneFB.h"

void setupApi(lua_State *L);
static LuaApplication *_gapp=NULL;

#if !defined(EGL_OPENGL_ES3_BIT_KHR)
#define EGL_OPENGL_ES3_BIT_KHR 0x0040
#endif

// EXT_texture_border_clamp
#ifndef GL_CLAMP_TO_BORDER
#define GL_CLAMP_TO_BORDER 0x812D
#endif

#ifndef GL_TEXTURE_BORDER_COLOR
#define GL_TEXTURE_BORDER_COLOR 0x1004
#endif

#if !defined(GL_EXT_multisampled_render_to_texture)
typedef void(GL_APIENTRY* PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)(
    GLenum target,
    GLsizei samples,
    GLenum internalformat,
    GLsizei width,
    GLsizei height);
typedef void(GL_APIENTRY* PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC)(
    GLenum target,
    GLenum attachment,
    GLenum textarget,
    GLuint texture,
    GLint level,
    GLsizei samples);
#endif

#if !defined(GL_OVR_multiview)
/// static const int GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_NUM_VIEWS_OVR       = 0x9630;
/// static const int GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_BASE_VIEW_INDEX_OVR = 0x9632;
/// static const int GL_MAX_VIEWS_OVR                                      = 0x9631;
typedef void(GL_APIENTRY* PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC)(
    GLenum target,
    GLenum attachment,
    GLuint texture,
    GLint level,
    GLint baseViewIndex,
    GLsizei numViews);
#endif

#if !defined(GL_OVR_multiview_multisampled_render_to_texture)
typedef void(GL_APIENTRY* PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVRPROC)(
    GLenum target,
    GLenum attachment,
    GLuint texture,
    GLint level,
    GLsizei samples,
    GLint baseViewIndex,
    GLsizei numViews);
#endif

#define DEBUG 1
#define OVR_LOG_TAG "VrGideros"

#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, OVR_LOG_TAG, __VA_ARGS__)
#if DEBUG
#define ALOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, OVR_LOG_TAG, __VA_ARGS__)
#else
#define ALOGV(...)
#endif

static const int CPU_LEVEL = 2;
static const int GPU_LEVEL = 3;
static const int NUM_MULTI_SAMPLES = 4;

#define MULTI_THREADED 0

/*
================================================================================

System Clock Time

================================================================================
*/

static double GetTimeInSeconds() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (now.tv_sec * 1e9 + now.tv_nsec) * 0.000000001;
}



#define ASSIGNV(a,b) a.x=b.x; a.y=b.y; a.z=b.z;
#define ASSIGNV4(a,b) a.x=b.x; a.y=b.y; a.z=b.z; a.w=b.w;
static bool roomEnabled=true,roomScreenEnabled=true,roomFloor=false;

struct ovrJava {
	JavaVM *Vm;
	JNIEnv *Env;
	jobject ActivityObject;
};

/*
================================================================================

ovrApp

================================================================================
*/

typedef struct {
    ovrJava Java;
    ANativeWindow* NativeWindow;
    bool Resumed;
    long long FrameIndex;
    double DisplayTime;
    int SwapInterval;
    int CpuLevel;
    int GpuLevel;
    int MainThreadTid;
    int RenderThreadTid;
    bool GamePadBackButtonDown;
    bool UseMultiview;
} ovrApp;

static void ovrApp_Clear(ovrApp* app) {
    app->Java.Vm = NULL;
    app->Java.Env = NULL;
    app->Java.ActivityObject = NULL;
    app->NativeWindow = NULL;
    app->Resumed = false;
    app->FrameIndex = 1;
    app->DisplayTime = 0;
    app->SwapInterval = 1;
    app->CpuLevel = 2;
    app->GpuLevel = 2;
    app->MainThreadTid = 0;
    app->RenderThreadTid = 0;
    app->GamePadBackButtonDown = false;
    app->UseMultiview = false;
}

static void ovrApp_HandleInput(ovrApp* app) {}

static int ovrApp_HandleKeyEvent(ovrApp* app, const int keyCode, const int action) {
    // Handle back button.
    if (keyCode == AKEYCODE_BACK || keyCode == AKEYCODE_BUTTON_B) {
        if (action == AKEY_EVENT_ACTION_DOWN) {
            app->GamePadBackButtonDown = true;
        } else if (action == AKEY_EVENT_ACTION_UP) {
            app->GamePadBackButtonDown = false;
        }
        return 1;
    }
    return 0;
}

/*
================================================================================

ovrMessageQueue

================================================================================
*/

typedef enum {
    MQ_WAIT_NONE, // don't wait
    MQ_WAIT_RECEIVED, // wait until the consumer thread has received the message
    MQ_WAIT_PROCESSED // wait until the consumer thread has processed the message
} ovrMQWait;

#define MAX_MESSAGE_PARMS 8
#define MAX_MESSAGES 1024

typedef struct {
    int Id;
    ovrMQWait Wait;
    long long Parms[MAX_MESSAGE_PARMS];
} ovrMessage;

static void ovrMessage_Init(ovrMessage* message, const int id, const int wait) {
    message->Id = id;
    message->Wait = (ovrMQWait) wait;
    memset(message->Parms, 0, sizeof(message->Parms));
}

static void ovrMessage_SetPointerParm(ovrMessage* message, int index, void* ptr) {
    *(void**)&message->Parms[index] = ptr;
}
static void* ovrMessage_GetPointerParm(ovrMessage* message, int index) {
    return *(void**)&message->Parms[index];
}
static void ovrMessage_SetIntegerParm(ovrMessage* message, int index, int value) {
    message->Parms[index] = value;
}
static int ovrMessage_GetIntegerParm(ovrMessage* message, int index) {
    return (int)message->Parms[index];
}
/// static void		ovrMessage_SetFloatParm( ovrMessage * message, int index, float value ) {
/// *(float *)&message->Parms[index] = value; } static float	ovrMessage_GetFloatParm( ovrMessage
/// * message, int index ) { return *(float *)&message->Parms[index]; }

// Cyclic queue with messages.
typedef struct {
    ovrMessage Messages[MAX_MESSAGES];
    volatile int Head; // dequeue at the head
    volatile int Tail; // enqueue at the tail
    ovrMQWait Wait;
    volatile bool EnabledFlag;
    volatile bool PostedFlag;
    volatile bool ReceivedFlag;
    volatile bool ProcessedFlag;
    pthread_mutex_t Mutex;
    pthread_cond_t PostedCondition;
    pthread_cond_t ReceivedCondition;
    pthread_cond_t ProcessedCondition;
} ovrMessageQueue;

static void ovrMessageQueue_Create(ovrMessageQueue* messageQueue) {
    messageQueue->Head = 0;
    messageQueue->Tail = 0;
    messageQueue->Wait = MQ_WAIT_NONE;
    messageQueue->EnabledFlag = false;
    messageQueue->PostedFlag = false;
    messageQueue->ReceivedFlag = false;
    messageQueue->ProcessedFlag = false;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&messageQueue->Mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    pthread_cond_init(&messageQueue->PostedCondition, NULL);
    pthread_cond_init(&messageQueue->ReceivedCondition, NULL);
    pthread_cond_init(&messageQueue->ProcessedCondition, NULL);
}

static void ovrMessageQueue_Destroy(ovrMessageQueue* messageQueue) {
    pthread_mutex_destroy(&messageQueue->Mutex);
    pthread_cond_destroy(&messageQueue->PostedCondition);
    pthread_cond_destroy(&messageQueue->ReceivedCondition);
    pthread_cond_destroy(&messageQueue->ProcessedCondition);
}

static void ovrMessageQueue_Enable(ovrMessageQueue* messageQueue, const bool set) {
    messageQueue->EnabledFlag = set;
}

static void ovrMessageQueue_PostMessage(ovrMessageQueue* messageQueue, const ovrMessage* message) {
    if (!messageQueue->EnabledFlag) {
        return;
    }
    while (messageQueue->Tail - messageQueue->Head >= MAX_MESSAGES) {
        usleep(1000);
    }
    pthread_mutex_lock(&messageQueue->Mutex);
    messageQueue->Messages[messageQueue->Tail & (MAX_MESSAGES - 1)] = *message;
    messageQueue->Tail++;
    messageQueue->PostedFlag = true;
    pthread_cond_broadcast(&messageQueue->PostedCondition);
    if (message->Wait == MQ_WAIT_RECEIVED) {
        while (!messageQueue->ReceivedFlag) {
            pthread_cond_wait(&messageQueue->ReceivedCondition, &messageQueue->Mutex);
        }
        messageQueue->ReceivedFlag = false;
    } else if (message->Wait == MQ_WAIT_PROCESSED) {
        while (!messageQueue->ProcessedFlag) {
            pthread_cond_wait(&messageQueue->ProcessedCondition, &messageQueue->Mutex);
        }
        messageQueue->ProcessedFlag = false;
    }
    pthread_mutex_unlock(&messageQueue->Mutex);
}

static void ovrMessageQueue_SleepUntilMessage(ovrMessageQueue* messageQueue) {
    if (messageQueue->Wait == MQ_WAIT_PROCESSED) {
        messageQueue->ProcessedFlag = true;
        pthread_cond_broadcast(&messageQueue->ProcessedCondition);
        messageQueue->Wait = MQ_WAIT_NONE;
    }
    pthread_mutex_lock(&messageQueue->Mutex);
    if (messageQueue->Tail > messageQueue->Head) {
        pthread_mutex_unlock(&messageQueue->Mutex);
        return;
    }
    while (!messageQueue->PostedFlag) {
        pthread_cond_wait(&messageQueue->PostedCondition, &messageQueue->Mutex);
    }
    messageQueue->PostedFlag = false;
    pthread_mutex_unlock(&messageQueue->Mutex);
}

static bool ovrMessageQueue_GetNextMessage(
    ovrMessageQueue* messageQueue,
    ovrMessage* message,
    bool waitForMessages) {
    if (messageQueue->Wait == MQ_WAIT_PROCESSED) {
        messageQueue->ProcessedFlag = true;
        pthread_cond_broadcast(&messageQueue->ProcessedCondition);
        messageQueue->Wait = MQ_WAIT_NONE;
    }
    if (waitForMessages) {
        ovrMessageQueue_SleepUntilMessage(messageQueue);
    }
    pthread_mutex_lock(&messageQueue->Mutex);
    if (messageQueue->Tail <= messageQueue->Head) {
        pthread_mutex_unlock(&messageQueue->Mutex);
        return false;
    }
    *message = messageQueue->Messages[messageQueue->Head & (MAX_MESSAGES - 1)];
    messageQueue->Head++;
    pthread_mutex_unlock(&messageQueue->Mutex);
    if (message->Wait == MQ_WAIT_RECEIVED) {
        messageQueue->ReceivedFlag = true;
        pthread_cond_broadcast(&messageQueue->ReceivedCondition);
    } else if (message->Wait == MQ_WAIT_PROCESSED) {
        messageQueue->Wait = MQ_WAIT_PROCESSED;
    }
    return true;
}

/*
================================================================================

ovrAppThread

================================================================================
*/

enum {
    MESSAGE_ON_CREATE,
    MESSAGE_ON_START,
    MESSAGE_ON_RESUME,
    MESSAGE_ON_PAUSE,
    MESSAGE_ON_STOP,
    MESSAGE_ON_DESTROY,
    MESSAGE_ON_SURFACE_CREATED,
    MESSAGE_ON_SURFACE_DESTROYED,
    MESSAGE_ON_KEY_EVENT,
    MESSAGE_ON_TOUCH_EVENT
};

typedef struct {
    JavaVM* JavaVm;
    jobject ActivityObject;
    pthread_t Thread;
    ovrMessageQueue MessageQueue;
    ANativeWindow* NativeWindow;
} ovrAppThread;

ovrApp appState;

#include "platformdata.h"
#include "platformplugin.h"
#include "openxr_program.h"
#include "graphicsplugin.h"
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

double lastTime=0;
double nextTime=0;
void StartOfFrame(XrFrameState &state)
{
    double curTime = GetTimeInSeconds();
    oculus::doTick(curTime-lastTime);
    lastTime=curTime;
    nextTime=lastTime+(state.predictedDisplayPeriod/1000000000.0);
}

void RenderEye(int eyeNum,float *vmat, float *pmat,int width,int height)
{
     oculus::doRender((eyeNum==1)?1:0,vmat,pmat,width,height,roomEnabled,roomScreenEnabled,roomFloor);
}

XrSpaceLocation headPos;
XrSpaceVelocity headSpeed;
void HandleInput(IOpenXrProgram::InputState *m_input,XrSpace m_appSpace,XrTime predictedDisplayTime,XrSpaceLocation *head,XrSpaceVelocity *headSpd)
{
	oculus::Input input;
	XrResult res;
	headPos=*head;
	headSpeed=*headSpd;
    for (auto hand : {Side::LEFT, Side::RIGHT}) {
        XrSpaceLocation spaceLocation{XR_TYPE_SPACE_LOCATION};
        res = xrLocateSpace(m_input->handSpace[hand], m_appSpace, predictedDisplayTime, &spaceLocation);
        CHECK_XRRESULT(res, "xrLocateSpace");
        if (XR_UNQUALIFIED_SUCCESS(res)) {
            if ((spaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0 &&
                (spaceLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0) {
                float scale = m_input->handScale[hand];
				input.deviceId=0+hand;
				input.deviceType=4; //Remote or Hand (32)
				input.caps=3|(hand?8:4);
				input.gripTrigger=m_input->handGrip[hand];
				input.indexTrigger=m_input->handIndex[hand];
				input.buttons=m_input->handButtons[hand];
				input.stickX=m_input->handStick[hand].x;
				input.stickY=m_input->handStick[hand].y;
				//ASSIGNV(input.pos,state.PointerPose.Position);
				//ASSIGNV4(input.rot,state.PointerPose.Orientation);

				input.poseStatus=spaceLocation.locationFlags;
				input.handScale=scale;
				ASSIGNV(input.pos,spaceLocation.pose.position);
				ASSIGNV4(input.rot,spaceLocation.pose.orientation);
				/*
				If hand, get bones
				for (int k=0;k<ovrHandBone_Max;k++) {
					ASSIGNV4(input.handBone[k],trackingState.BoneRotations[k]);
				}*/
				oculus::doInputEvent(input);
            }
        } else {
            // Tracking loss is expected when the hand is not active so only log a message
            // if the hand is active.
            if (m_input->handActive[hand] == XR_TRUE) {
                const char* handName[] = {"left", "right"};
                Log::Write(Log::Level::Verbose,
                           Fmt("Unable to locate %s hand action space in app space: %d", handName[hand], res));
            }
        }
    }
}

std::map<std::string,bool> availableExtensions;
std::map<std::string,bool> enabledExtensions;

std::shared_ptr<IOpenXrProgram> ProgramInstance=NULL;
void* AppThreadFunction(void* parm) {
    ovrAppThread* appThread = (ovrAppThread*)parm;
    bool LuaInitialized=false;

    std::shared_ptr<Options> options = std::make_shared<Options>();
    std::shared_ptr<PlatformData> data = std::make_shared<PlatformData>();
    data->applicationVM = appThread->JavaVm;
    data->applicationActivity = appThread->ActivityObject;

    options->GraphicsPlugin = "OpenGLES";

    ovrJava java;
    java.Vm = appThread->JavaVm;
    java.Vm->AttachCurrentThread(&java.Env,NULL);
    java.ActivityObject = appThread->ActivityObject;

    // Note that AttachCurrentThread will reset the thread name.
    prctl(PR_SET_NAME, (long)"OVR::Main", 0, 0, 0);

    // Create platform-specific implementation.
    std::shared_ptr<IPlatformPlugin> platformPlugin = CreatePlatformPlugin(options, data);
    // Create graphics API implementation.
    std::shared_ptr<IGraphicsPlugin> graphicsPlugin = CreateGraphicsPlugin(options, platformPlugin);
    graphicsPlugin->RenderEye=RenderEye;

    // Initialize the OpenXR program.
    std::shared_ptr<IOpenXrProgram> program = CreateOpenXrProgram(options, platformPlugin, graphicsPlugin);
    program->HandleInput=HandleInput;
    program->StartOfFrame=StartOfFrame;

    // Initialize the loader for this platform
    PFN_xrInitializeLoaderKHR initializeLoader = nullptr;
    if (XR_SUCCEEDED(
            xrGetInstanceProcAddr(XR_NULL_HANDLE, "xrInitializeLoaderKHR", (PFN_xrVoidFunction*)(&initializeLoader)))) {
        XrLoaderInitInfoAndroidKHR loaderInitInfoAndroid = {XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR};
        loaderInitInfoAndroid.applicationVM = data->applicationVM;
        loaderInitInfoAndroid.applicationContext = java.ActivityObject;
        initializeLoader((const XrLoaderInitInfoBaseHeaderKHR*)&loaderInitInfoAndroid);
    }

    availableExtensions=program->ProbeExtensions();

    std::vector<std::string> extraExtensions;

    extraExtensions.push_back(XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME);
    if (availableExtensions[XR_FB_PASSTHROUGH_EXTENSION_NAME]) {
        ALOGV("EXT: Using passthrough");
    	extraExtensions.push_back(XR_FB_PASSTHROUGH_EXTENSION_NAME);
    }
    if (availableExtensions[XR_FB_TRIANGLE_MESH_EXTENSION_NAME]) {
        ALOGV("EXT: Using mesh");
    	extraExtensions.push_back(XR_FB_TRIANGLE_MESH_EXTENSION_NAME);
    }
    if (availableExtensions[XR_EXT_HAND_TRACKING_EXTENSION_NAME]) {
        ALOGV("EXT: Using hand tracking");
    	extraExtensions.push_back(XR_EXT_HAND_TRACKING_EXTENSION_NAME);
    	extraExtensions.push_back(XR_FB_HAND_TRACKING_MESH_EXTENSION_NAME);
    	extraExtensions.push_back(XR_FB_HAND_TRACKING_AIM_EXTENSION_NAME);
    	extraExtensions.push_back(XR_FB_HAND_TRACKING_CAPSULES_EXTENSION_NAME);
    }
    if (availableExtensions[XR_FB_SCENE_EXTENSION_NAME]) {
        ALOGV("EXT: Using scene");
        extraExtensions.push_back(XR_FB_SPATIAL_ENTITY_EXTENSION_NAME);
   		extraExtensions.push_back(XR_FB_SPATIAL_ENTITY_QUERY_EXTENSION_NAME);
		extraExtensions.push_back(XR_FB_SPATIAL_ENTITY_STORAGE_EXTENSION_NAME);
		extraExtensions.push_back(XR_FB_SPATIAL_ENTITY_CONTAINER_EXTENSION_NAME);
		extraExtensions.push_back(XR_FB_SCENE_EXTENSION_NAME);
		extraExtensions.push_back(XR_FB_SCENE_CAPTURE_EXTENSION_NAME);
    }


    program->CreateInstance(extraExtensions);
    enabledExtensions=program->EnabledExtensions();

    if (enabledExtensions[XR_FB_PASSTHROUGH_EXTENSION_NAME])
    	program->AddExtension(new PassthroughFB());

    if (enabledExtensions[XR_EXT_HAND_TRACKING_EXTENSION_NAME])
    	program->AddExtension(new HandTrackingFB());

    if (enabledExtensions[XR_FB_SCENE_EXTENSION_NAME])
    	program->AddExtension(new SceneFB());

    program->InitializeSystem();

    options->SetEnvironmentBlendMode(program->GetPreferredBlendMode());
    platformPlugin->UpdateOptions(options);
    graphicsPlugin->UpdateOptions(options);

    program->InitializeDevice();
    program->InitializeSession();
    program->CreateSwapchains();

     appState.CpuLevel = CPU_LEVEL;
    appState.GpuLevel = GPU_LEVEL;
    appState.MainThreadTid = gettid();
    appState.DisplayTime=0;

    bool requestRestart = false;
    ProgramInstance=program;
	for (auto it=ProgramInstance->VRExts.begin();it!=ProgramInstance->VRExts.end();it++)
		(*it)->Setup(program);


    for (bool destroyed = false; destroyed == false;) {
        for (;;) {
            ovrMessage message;
            const bool waitForMessages = false;
            if (!ovrMessageQueue_GetNextMessage(
                    &appThread->MessageQueue, &message, waitForMessages)) {
                break;
            }

            switch (message.Id) {
                case MESSAGE_ON_CREATE: {
                    break;
                }
                case MESSAGE_ON_START: {
                    break;
                }
                case MESSAGE_ON_RESUME: {
                    appState.Resumed = true;
                    break;
                }
                case MESSAGE_ON_PAUSE: {
                    appState.Resumed = false;
                    break;
                }
                case MESSAGE_ON_STOP: {
                    break;
                }
                case MESSAGE_ON_DESTROY: {
                    appState.NativeWindow = NULL;
                    destroyed = true;
                    break;
                }
                case MESSAGE_ON_SURFACE_CREATED: {
                    appState.NativeWindow = (ANativeWindow*)ovrMessage_GetPointerParm(&message, 0);
                    if (!LuaInitialized) {
                        roomEnabled=true;
                        roomScreenEnabled=true;
						_gapp->initialize();
						setupApi(LuaDebugging::L);
						LuaInitialized=true;
                    }
                    break;
                }
                case MESSAGE_ON_SURFACE_DESTROYED: {
                    appState.NativeWindow = NULL;
                    break;
                }
                case MESSAGE_ON_KEY_EVENT: {
                    ovrApp_HandleKeyEvent(
                        &appState,
                        ovrMessage_GetIntegerParm(&message, 0),
                        ovrMessage_GetIntegerParm(&message, 1));
                    break;
                }
            }

            //ovrApp_HandleVrModeChanges(&appState);
        }

        // We must read from the event queue with regular frequency.
        //ovrApp_HandleVrApiEvents(&appState);

        ovrApp_HandleInput(&appState);

        // This is the only place the frame index is incremented, right before
        // calling vrapi_GetPredictedDisplayTime().
        appState.FrameIndex++;

        bool exitRenderLoop = false;
		program->PollEvents(&exitRenderLoop, &requestRestart);
		if (exitRenderLoop) {
			break;
		}

		if (program->IsSessionRunning()&&appState.NativeWindow) {
			program->PollActions();
			program->SetViewSpace(roomFloor?"Stage":"Local");
			program->RenderFrame();
		    oculus::doGLQueue(0);
		    double rTime = (nextTime-GetTimeInSeconds())*1000000000;
		    if (rTime>0) {
		    	size_t nst=rTime;
			    oculus::doGLQueue(nst);
			}
		} else {
			// Throttle loop since xrWaitFrame won't be called.
			std::this_thread::sleep_for(std::chrono::milliseconds(33));
		}

    }

    ProgramInstance=NULL;

    if (LuaInitialized)
    	_gapp->deinitialize();

    java.Vm->DetachCurrentThread();

    return NULL;
}

static void ovrAppThread_Create(ovrAppThread* appThread, JNIEnv* env, jobject activityObject) {
    env->GetJavaVM(&appThread->JavaVm);
    appThread->ActivityObject = env->NewGlobalRef(activityObject);
    appThread->Thread = 0;
    appThread->NativeWindow = NULL;
    ovrMessageQueue_Create(&appThread->MessageQueue);
/*
    const int createErr = pthread_create(&appThread->Thread, NULL, AppThreadFunction, appThread);
    if (createErr != 0) {
        ALOGE("pthread_create returned %i", createErr);
    }*/
}

static void ovrAppThread_Destroy(ovrAppThread* appThread, JNIEnv* env) {
    //pthread_join(appThread->Thread, NULL);
    env->DeleteGlobalRef(appThread->ActivityObject);
    ovrMessageQueue_Destroy(&appThread->MessageQueue);
}

/*
================================================================================

Activity lifecycle

================================================================================
*/

static ovrAppThread *appThread=NULL;
void oculus::onCreate(JNIEnv *env,jobject activity,LuaApplication *app) {
    ALOGV("    GLES3JNILib::onCreate()");
    _gapp=app;

    appThread = (ovrAppThread*)malloc(sizeof(ovrAppThread));
    ovrAppThread_Create(appThread, env, activity);
}

void oculus::runThread() {
	AppThreadFunction(appThread);
}

void oculus::postCreate() {
    ovrMessageQueue_Enable(&appThread->MessageQueue, true);
    ovrMessage message;
    ovrMessage_Init(&message, MESSAGE_ON_CREATE, MQ_WAIT_PROCESSED);
    ovrMessageQueue_PostMessage(&appThread->MessageQueue, &message);
}

void oculus::onStart()
{
    ALOGV("    GLES3JNILib::onStart()");
    ovrMessage message;
    ovrMessage_Init(&message, MESSAGE_ON_START, MQ_WAIT_PROCESSED);
    ovrMessageQueue_PostMessage(&appThread->MessageQueue, &message);
}

void oculus::onResume()
{
    ALOGV("    GLES3JNILib::onResume()");
    ovrMessage message;
    ovrMessage_Init(&message, MESSAGE_ON_RESUME, MQ_WAIT_PROCESSED);
    ovrMessageQueue_PostMessage(&appThread->MessageQueue, &message);
}

void oculus::onPause()
{
    ALOGV("    GLES3JNILib::onPause()");
    ovrMessage message;
    ovrMessage_Init(&message, MESSAGE_ON_PAUSE, MQ_WAIT_PROCESSED);
    ovrMessageQueue_PostMessage(&appThread->MessageQueue, &message);
}

void oculus::onStop()
{
    ALOGV("    GLES3JNILib::onStop()");
    ovrMessage message;
    ovrMessage_Init(&message, MESSAGE_ON_STOP, MQ_WAIT_PROCESSED);
    ovrMessageQueue_PostMessage(&appThread->MessageQueue, &message);
}

void oculus::onDestroy(JNIEnv *env)
{
    ALOGV("    GLES3JNILib::onDestroy()");
    ovrMessage message;
    ovrMessage_Init(&message, MESSAGE_ON_DESTROY, MQ_WAIT_PROCESSED);
    ovrMessageQueue_PostMessage(&appThread->MessageQueue, &message);
    ovrMessageQueue_Enable(&appThread->MessageQueue, false);

    ovrAppThread_Destroy(appThread, env);
    free(appThread);
    appThread=NULL;
}

/*
================================================================================

Surface lifecycle

================================================================================
*/

void oculus::onSurfaceCreated(JNIEnv *env,jobject surface)
{
    ALOGV("    GLES3JNILib::onSurfaceCreated()");

    ANativeWindow* newNativeWindow = ANativeWindow_fromSurface(env, surface);
    if (ANativeWindow_getWidth(newNativeWindow) < ANativeWindow_getHeight(newNativeWindow)) {
        // An app that is relaunched after pressing the home button gets an initial surface with
        // the wrong orientation even though android:screenOrientation="landscape" is set in the
        // manifest. The choreographer callback will also never be called for this surface because
        // the surface is immediately replaced with a new surface with the correct orientation.
        ALOGE("        Surface not in landscape mode!");
    }

    ALOGV("        NativeWindow = ANativeWindow_fromSurface( env, surface )");
    appThread->NativeWindow = newNativeWindow;
    ovrMessage message;
    ovrMessage_Init(&message, MESSAGE_ON_SURFACE_CREATED, MQ_WAIT_PROCESSED);
    ovrMessage_SetPointerParm(&message, 0, appThread->NativeWindow);
    ovrMessageQueue_PostMessage(&appThread->MessageQueue, &message);
}

void oculus::onSurfaceChanged(JNIEnv *env,jobject surface)
{
    ALOGV("    GLES3JNILib::onSurfaceChanged()");

    ANativeWindow* newNativeWindow = ANativeWindow_fromSurface(env, surface);
    if (ANativeWindow_getWidth(newNativeWindow) < ANativeWindow_getHeight(newNativeWindow)) {
        // An app that is relaunched after pressing the home button gets an initial surface with
        // the wrong orientation even though android:screenOrientation="landscape" is set in the
        // manifest. The choreographer callback will also never be called for this surface because
        // the surface is immediately replaced with a new surface with the correct orientation.
        ALOGE("        Surface not in landscape mode!");
    }

    if (newNativeWindow != appThread->NativeWindow) {
/*        if (appThread->NativeWindow != NULL) {
            ovrMessage message;
            ovrMessage_Init(&message, MESSAGE_ON_SURFACE_DESTROYED, MQ_WAIT_PROCESSED);
            ovrMessageQueue_PostMessage(&appThread->MessageQueue, &message);
            ALOGV("        ANativeWindow_release( NativeWindow )");
            ANativeWindow_release(appThread->NativeWindow);
            appThread->NativeWindow = NULL;
        }
        if (newNativeWindow != NULL) {
            ALOGV("        NativeWindow = ANativeWindow_fromSurface( env, surface )");
            appThread->NativeWindow = newNativeWindow;
            ovrMessage message;
            ovrMessage_Init(&message, MESSAGE_ON_SURFACE_CREATED, MQ_WAIT_PROCESSED);
            ovrMessage_SetPointerParm(&message, 0, appThread->NativeWindow);
            ovrMessageQueue_PostMessage(&appThread->MessageQueue, &message);
        }*/
    } else if (newNativeWindow != NULL) {
        ANativeWindow_release(newNativeWindow);
    }
}

void oculus::onSurfaceDestroyed()
{
	ALOGV("    GLES3JNILib::onSurfaceDestroyed()");
    ovrMessage message;
    ovrMessage_Init(&message, MESSAGE_ON_SURFACE_DESTROYED, MQ_WAIT_PROCESSED);
    ovrMessageQueue_PostMessage(&appThread->MessageQueue, &message);
    ALOGV("        ANativeWindow_release( NativeWindow )");
    ANativeWindow_release(appThread->NativeWindow);
    appThread->NativeWindow = NULL;
}

void oculus::onLuaReinit() {
	setupApi(LuaDebugging::L);
}

static int enableRoom(lua_State *L) {
	roomEnabled=lua_toboolean(L,1);
	roomScreenEnabled=lua_toboolean(L,2);
	return 0;
}

static int getExtensions(lua_State *L) {
	const std::map<std::string,bool> &e=lua_toboolean(L,1)?availableExtensions:enabledExtensions;
	lua_newtable(L);
	for (auto it=e.begin();it!=e.end();it++)
	{
		lua_pushstring(L,it->first.c_str());
		lua_pushboolean(L,it->second);
		lua_rawset(L,-3);
	}
	return 1;
}


static int setTrackingSpace(lua_State *L) {
	//0: LOCAL
	//3: STAGE
	int s=luaL_optinteger(L,1,0);
	roomFloor=(s==3);
	return 0;
}

static void pushVector(lua_State *L,XrVector3f v) {
	lua_newtable(L);
	lua_pushnumber(L,v.x); lua_rawseti(L,-2,1);
	lua_pushnumber(L,v.y); lua_rawseti(L,-2,2);
	lua_pushnumber(L,v.z); lua_rawseti(L,-2,3);
}

static void pushVector4(lua_State *L,XrQuaternionf v) {
	lua_newtable(L);
	lua_pushnumber(L,v.x); lua_rawseti(L,-2,1);
	lua_pushnumber(L,v.y); lua_rawseti(L,-2,2);
	lua_pushnumber(L,v.z); lua_rawseti(L,-2,3);
	lua_pushnumber(L,v.w); lua_rawseti(L,-2,4);
}

static int getHeadPose(lua_State *L) {
	lua_newtable(L);
	//Pose
	lua_pushinteger(L,headPos.locationFlags);
	lua_setfield(L, -2,	"poseStatus");
	pushVector(L,headPos.pose.position);
	lua_setfield(L, -2, "position");
	pushVector4(L,headPos.pose.orientation);
	lua_setfield(L, -2, "rotation");
	pushVector(L,headSpeed.linearVelocity);
	lua_setfield(L, -2, "linearVelocity");
	pushVector(L,headSpeed.angularVelocity);
	lua_setfield(L, -2, "angularVelocity");
	/*
	pushVector(L,last_Head.HeadPose.LinearAcceleration);
	lua_setfield(L, -2, "linearAcceleration");
	pushVector(L,last_Head.HeadPose.AngularAcceleration);
	lua_setfield(L, -2, "angularAcceleration"); */
	return 1;
}

static int getHandMesh(lua_State *L) {
	/*if (appState.Ovr) {
		ovrHandMesh *mesh=new ovrHandMesh;
		mesh->Header.Version=ovrHandVersion_1;
		if (vrapi_GetHandMesh(appState.Ovr, (ovrHandedness) luaL_checkinteger(L,1), &mesh->Header)>=0)
		{
			lua_newtable(L);
			lua_newtable(L);
			for (int k=0;k<mesh->NumIndices;k++)
			{
				lua_pushinteger(L,mesh->Indices[k]+1);
				lua_rawseti(L,-2,k+1);
			}
			lua_setfield(L,-2,"indices");

			lua_newtable(L);
			for (int k=0;k<mesh->NumVertices;k++)
			{
				lua_pushnumber(L,mesh->VertexPositions[k].x);
				lua_rawseti(L,-2,k*3+1);
				lua_pushnumber(L,mesh->VertexPositions[k].y);
				lua_rawseti(L,-2,k*3+2);
				lua_pushnumber(L,mesh->VertexPositions[k].z);
				lua_rawseti(L,-2,k*3+3);
			}
			lua_setfield(L,-2,"vertices");

			lua_newtable(L);
			for (int k=0;k<mesh->NumVertices;k++)
			{
				lua_pushnumber(L,mesh->VertexNormals[k].x);
				lua_rawseti(L,-2,k*3+1);
				lua_pushnumber(L,mesh->VertexNormals[k].y);
				lua_rawseti(L,-2,k*3+2);
				lua_pushnumber(L,mesh->VertexNormals[k].z);
				lua_rawseti(L,-2,k*3+3);
			}
			lua_setfield(L,-2,"normals");

			lua_newtable(L);
			for (int k=0;k<mesh->NumVertices;k++)
			{
				lua_pushnumber(L,mesh->VertexUV0[k].x);
				lua_rawseti(L,-2,k*2+1);
				lua_pushnumber(L,mesh->VertexUV0[k].y);
				lua_rawseti(L,-2,k*2+2);
			}
			lua_setfield(L,-2,"texcoords");

			lua_newtable(L);
			lua_newtable(L);
			for (int k=0;k<mesh->NumVertices;k++)
			{
				lua_pushinteger(L,mesh->BlendIndices[k].x);
				lua_rawseti(L,-2,k*4+1);
				lua_pushinteger(L,mesh->BlendIndices[k].y);
				lua_rawseti(L,-2,k*4+2);
				lua_pushinteger(L,mesh->BlendIndices[k].z);
				lua_rawseti(L,-2,k*4+3);
				lua_pushinteger(L,mesh->BlendIndices[k].w);
				lua_rawseti(L,-2,k*4+4);
			}
			lua_setfield(L,-2,"bi");

			lua_newtable(L);
			for (int k=0;k<mesh->NumVertices;k++)
			{
				lua_pushnumber(L,mesh->BlendWeights[k].x);
				lua_rawseti(L,-2,k*4+1);
				lua_pushnumber(L,mesh->BlendWeights[k].y);
				lua_rawseti(L,-2,k*4+2);
				lua_pushnumber(L,mesh->BlendWeights[k].z);
				lua_rawseti(L,-2,k*4+3);
				lua_pushnumber(L,mesh->BlendWeights[k].w);
				lua_rawseti(L,-2,k*4+4);
			}
			lua_setfield(L,-2,"bw");
			lua_setfield(L,-2,"animdata");
			delete mesh;
			return 1;
		}
		delete mesh;
	}*/
	return 0;
}

static int getHandSkeleton(lua_State *L) {
	/*
	if (appState.Ovr) {
		ovrHandSkeleton *mesh=new ovrHandSkeleton;
		mesh->Header.Version=ovrHandVersion_1;
		if (vrapi_GetHandSkeleton(appState.Ovr, (ovrHandedness) luaL_checkinteger(L,1), &mesh->Header)>=0) {
			lua_newtable(L);
			lua_newtable(L);
			for (int k=0;k<mesh->NumCapsules;k++)
			{
				lua_newtable(L);
				lua_pushinteger(L,mesh->Capsules[k].BoneIndex);
				lua_setfield(L, -2, "boneIndex");
				pushVector(L,mesh->Capsules[k].Points[0]);
				lua_setfield(L, -2, "pointA");
				pushVector(L,mesh->Capsules[k].Points[1]);
				lua_setfield(L, -2, "pointB");
				lua_pushnumber(L,mesh->Capsules[k].Radius);
				lua_setfield(L, -2, "radius");
				lua_rawseti(L,-2,k+1);
			}
			lua_setfield(L,-2,"capsules");

			lua_newtable(L);
			for (int k=0;k<mesh->NumBones;k++)
			{
				lua_pushinteger(L,mesh->BoneParentIndices[k]);
				lua_rawseti(L,-2,k+1);
			}
			lua_setfield(L,-2,"boneParents");

			lua_newtable(L);
			for (int k=0;k<mesh->NumBones;k++)
			{
				lua_newtable(L);
				pushVector(L,mesh->BonePoses[k].Position);
				lua_setfield(L, -2, "position");
				pushVector4(L,mesh->BonePoses[k].Orientation);
				lua_setfield(L, -2, "rotation");
				lua_rawseti(L,-2,k+1);
			}
			lua_setfield(L,-2,"bones");
			delete mesh;

			return 1;
		}
		delete mesh;
	}*/
	return 0;
}

void setupApi(lua_State *L)
{
	roomEnabled=true;
	roomScreenEnabled=true;
	roomFloor=false;
	/*if (appState.Ovr)
		vrapi_SetTrackingSpace(appState.Ovr,VRAPI_TRACKING_SPACE_LOCAL); */
    static const luaL_Reg functionList[] = {
		{"enableRoom", enableRoom},
		{"setTrackingSpace", setTrackingSpace},
		{"getHeadPose", getHeadPose},
		{"getHandMesh", getHandMesh},
		{"getHandSkeleton", getHandSkeleton},
		{"getExtensions", getExtensions},
        {NULL, NULL},
    };

    lua_newtable(L);
    luaL_register(L, NULL, functionList);
    lua_setglobal(L, "Oculus");
    if (ProgramInstance)
    	for (auto it=ProgramInstance->VRExts.begin();it!=ProgramInstance->VRExts.end();it++)
    		(*it)->SetupAPI(L);
	ALOGV("=== API registered");
}
