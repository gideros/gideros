#ifndef GPLUGIN_H
#define GPLUGIN_H

#include "gexport.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
int g_registerPlugin(void*(*main)(lua_State*, int));
#ifdef __cplusplus
}
#endif

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#ifdef _WIN32
#define G_DLLEXPORT __declspec(dllexport)
#elif __EMSCRIPTEN__
#include <emscripten.h>
#define G_DLLEXPORT EMSCRIPTEN_KEEPALIVE
#else
#define G_DLLEXPORT
#endif


#define REGISTER_PLUGIN_STATIC_C(name, version) void REGISTER_PLUGIN_CANNOT_BE_USED_IN_C_OR_OBJC;

#define REGISTER_PLUGIN_DYNAMIC_C(name, version) \
	G_DLLEXPORT void* g_pluginMain(lua_State* L, int type) \
	{ \
		if (type == 0) \
			g_initializePlugin(L); \
		else if (type == 1) \
			g_deinitializePlugin(L); \
		else if (type == 2) \
			return (void*)name; \
		else if (type == 3) \
			return (void*)version; \
		return NULL; \
	}

#define REGISTER_PLUGIN_DYNAMICNAMED_C(name, version, symbol) \
	G_DLLEXPORT void* g_pluginMain_##symbol(lua_State* L, int type) \
	{ \
		if (type == 0) \
			g_initializePlugin(L); \
		else if (type == 1) \
			g_deinitializePlugin(L); \
		else if (type == 2) \
			return (void*)name; \
		else if (type == 3) \
			return (void*)version; \
		return NULL; \
	}


#define REGISTER_PLUGIN_STATIC_CPP(name, version) \
	extern "C" { \
	static void* g_pluginMain(lua_State* L, int type) \
	{ \
		if (type == 0) \
			g_initializePlugin(L); \
		else if (type == 1) \
			g_deinitializePlugin(L); \
		else if (type == 2) \
			return (void*)name; \
		else if (type == 3) \
			return (void*)version; \
		return NULL; \
	} \
	static int g_temp = g_registerPlugin(g_pluginMain); \
	}

#define REGISTER_PLUGIN_STATICNAMED_CPP(name, version, symbol) \
	extern "C" { \
	void* g_pluginMain_##symbol(lua_State* L, int type) \
	{ \
		if (type == 0) \
			g_initializePlugin(L); \
				else if (type == 1) \
			g_deinitializePlugin(L); \
				else if (type == 2) \
			return (void*)name; \
				else if (type == 3) \
			return (void*)version; \
		return NULL; \
	} \
	}

#define REGISTER_PLUGIN_DYNAMIC_CPP(name, version) \
	extern "C" { \
	G_DLLEXPORT void* g_pluginMain(lua_State* L, int type) \
	{ \
		if (type == 0) \
			g_initializePlugin(L); \
		else if (type == 1) \
			g_deinitializePlugin(L); \
		else if (type == 2) \
			return (void*)name; \
		else if (type == 3) \
			return (void*)version; \
		return NULL; \
	} \
	}

#define REGISTER_PLUGIN_DYNAMICNAMED_CPP(name, version, symbol) \
	extern "C" { \
	G_DLLEXPORT void* g_pluginMain_##symbol(lua_State* L, int type) \
	{ \
		if (type == 0) \
			g_initializePlugin(L); \
		else if (type == 1) \
			g_deinitializePlugin(L); \
		else if (type == 2) \
			return (void*)name; \
		else if (type == 3) \
			return (void*)version; \
		return NULL; \
	} \
	}

#if __ANDROID__
#include <jni.h>
#endif

#define REGISTER_PLUGIN_ANDROID_C(name, version) \
	G_DLLEXPORT void* g_pluginMain(lua_State* L, int type) \
	{ \
		if (type == 0) \
			g_initializePlugin(L); \
		else if (type == 1) \
			g_deinitializePlugin(L); \
		else if (type == 2) \
			return (void*)name; \
		else if (type == 3) \
			return (void*)version; \
		return NULL; \
	} \
	JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) \
	{ \
		g_registerPlugin(g_pluginMain); \
		return JNI_VERSION_1_6; \
	}
	
#define REGISTER_PLUGIN_ANDROID_CPP(name, version) \
	extern "C" { \
	G_DLLEXPORT void* g_pluginMain(lua_State* L, int type) \
	{ \
		if (type == 0) \
			g_initializePlugin(L); \
		else if (type == 1) \
			g_deinitializePlugin(L); \
		else if (type == 2) \
			return (void*)name; \
		else if (type == 3) \
			return (void*)version; \
		return NULL; \
	} \
	JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) \
	{ \
		g_registerPlugin(g_pluginMain); \
		return JNI_VERSION_1_6; \
	} \
	}
	
#ifdef __cplusplus
#define REGISTER_PLUGIN_STATICNAMED(name, version, symbol) REGISTER_PLUGIN_STATICNAMED_CPP(name, version, symbol)
#define REGISTER_PLUGIN_DYNAMICNAMED(name, version, symbol) REGISTER_PLUGIN_DYNAMICNAMED_CPP(name, version, symbol)
#define REGISTER_PLUGIN_STATIC(name, version) REGISTER_PLUGIN_STATIC_CPP(name, version)
#define REGISTER_PLUGIN_DYNAMIC(name, version) REGISTER_PLUGIN_DYNAMIC_CPP(name, version)
#define REGISTER_PLUGIN_ANDROID(name, version) REGISTER_PLUGIN_ANDROID_CPP(name, version)
#else
#define REGISTER_PLUGIN_STATICNAMED(name, version, symbol) REGISTER_PLUGIN_STATICNAMED_C(name, version, symbol)
#define REGISTER_PLUGIN_DYNAMICNAMED(name, version, symbol) REGISTER_PLUGIN_DYNAMICNAMED_C(name, version, symbol)
#define REGISTER_PLUGIN_STATIC(name, version) REGISTER_PLUGIN_STATIC_C(name, version)
#define REGISTER_PLUGIN_DYNAMIC(name, version) REGISTER_PLUGIN_DYNAMIC_C(name, version)
#define REGISTER_PLUGIN_ANDROID(name, version) REGISTER_PLUGIN_ANDROID_C(name, version)
#endif

#ifdef QT_NO_DEBUG 
#define REGISTER_PLUGIN(name, version) REGISTER_PLUGIN_DYNAMIC(name, version)
#define REGISTER_PLUGIN_NAMED(name, version, symbol) REGISTER_PLUGIN_DYNAMIC(name, version)
#elif TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR || TARGET_OS_OSX
#define REGISTER_PLUGIN(name, version) REGISTER_PLUGIN_STATIC(name, version)
#define REGISTER_PLUGIN_NAMED(name, version, symbol) REGISTER_PLUGIN_STATIC(name, version)
#elif __ANDROID__
#define REGISTER_PLUGIN(name, version) REGISTER_PLUGIN_ANDROID(name, version)
#define REGISTER_PLUGIN_NAMED(name, version, symbol) REGISTER_PLUGIN_STATIC(name, version)
#elif WINSTORE
#define REGISTER_PLUGIN(name, version) REGISTER_PLUGIN_STATIC(name, version)
#define REGISTER_PLUGIN_NAMED(name, version, symbol) REGISTER_PLUGIN_STATIC(name, version)
#elif __EMSCRIPTEN__
#define REGISTER_PLUGIN(name, version) REGISTER_PLUGIN_STATIC(name, version)
#define REGISTER_PLUGIN_NAMED(name, version, symbol) REGISTER_PLUGIN_DYNAMICNAMED(name, version, symbol)
#else
#define REGISTER_PLUGIN(name, version) REGISTER_PLUGIN_DYNAMIC(name, version)
#define REGISTER_PLUGIN_NAMED(name, version, symbol) REGISTER_PLUGIN_DYNAMIC(name, version)
#endif

#define IMPORT_PLUGIN(symbol) \
extern "C" void* g_pluginMain_##symbol(lua_State* L, int type);\
static int g_pluginRef_##symbol = g_registerPlugin(g_pluginMain_##symbol);

#ifdef __cplusplus
extern "C" {
#endif

GIDEROS_API void g_disableTypeChecking();
GIDEROS_API void g_enableTypeChecking();
GIDEROS_API int g_isTypeCheckingEnabled();
GIDEROS_API void g_createClass(lua_State* L,
								 const char* classname,
								 const char* basename,
								 int (*constructor) (lua_State*),
								 int (*destructor) (lua_State*),
								 const luaL_reg* functionlist);
GIDEROS_API void g_pushInstance(lua_State* L, const char* classname, void* ptr);
GIDEROS_API void* g_getInstance(lua_State* L, const char* classname, int index);
GIDEROS_API void g_setInstance(lua_State* L, int index, void* ptr);
GIDEROS_API int g_isInstanceOf(lua_State* L, const char* classname, int index);

GIDEROS_API int g_error(lua_State* L, const char* msg);


#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR || TARGET_OS_OSX
#ifdef __OBJC__
#if TARGET_OS_OSX
#define UIViewController NSViewController
#endif
@class UIViewController;
UIViewController* g_getRootViewController();
#endif
#endif

#if __ANDROID__
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
#endif

GIDEROS_API void g_registerOpenUrlCallback(void(*openUrl)(lua_State*, const char *));
GIDEROS_API void g_registerEnterFrameCallback(void(*enterFrame)(lua_State*));
GIDEROS_API void g_registerSuspendCallback(void(*suspend)(lua_State*));
GIDEROS_API void g_registerResumeCallback(void(*resume)(lua_State*));
GIDEROS_API void g_registerForegroundCallback(void(*foreground)(lua_State*));
GIDEROS_API void g_registerBackgroundCallback(void(*background)(lua_State*));

#ifdef __cplusplus
}
#endif



#endif
