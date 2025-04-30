#include "gflurry.h"
#include <jni.h>
#include <stdlib.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

static jobject parametersToMap(const char **parameters)
{
    if (parameters == NULL)
        return NULL;

	JNIEnv *env = g_getJNIEnv();
	
	jclass cls = env->FindClass("java/util/HashMap");	

	jobject jmapobj = env->NewObject(cls, env->GetMethodID(cls, "<init>", "()V"));
	
	while (*parameters && *(parameters + 1))
    {
		jstring jKey = env->NewStringUTF(*parameters);
		jstring jVal = env->NewStringUTF(*(parameters + 1));
		env->CallObjectMethod(jmapobj, env->GetMethodID(cls, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;"), jKey, jVal);
		env->DeleteLocalRef(jKey);
		env->DeleteLocalRef(jVal);	
		parameters += 2;
	}

	env->DeleteLocalRef(cls);

	return jmapobj;	
}

extern "C" {

void gflurry_StartSession(const char *apiKey)
{
	JNIEnv *env = g_getJNIEnv();

	jclass cls = env->FindClass("com/giderosmobile/android/plugins/flurry/GFlurry");	
	jstring japiKey = env->NewStringUTF(apiKey);
	env->CallStaticVoidMethod(cls, env->GetStaticMethodID(cls, "startSession", "(Ljava/lang/String;)V"), japiKey);
	env->DeleteLocalRef(japiKey);
	env->DeleteLocalRef(cls);
}

void gflurry_LogEvent(const char *eventName, const char **parameters, int timed)
{
	JNIEnv *env = g_getJNIEnv();

	jclass cls = env->FindClass("com/giderosmobile/android/plugins/flurry/GFlurry");	

    jstring eventName2 = env->NewStringUTF(eventName);

    jobject parameters2 = parametersToMap(parameters);
    
    if (parameters2 && timed)
		env->CallStaticVoidMethod(cls, env->GetStaticMethodID(cls, "logEvent", "(Ljava/lang/String;Ljava/util/Map;Z)V"), eventName2, parameters2, (jboolean)1);
    else if (!parameters2 && timed)
		env->CallStaticVoidMethod(cls, env->GetStaticMethodID(cls, "logEvent", "(Ljava/lang/String;Z)V"), eventName2, (jboolean)1);
    else if (parameters2 && !timed)
		env->CallStaticVoidMethod(cls, env->GetStaticMethodID(cls, "logEvent", "(Ljava/lang/String;Ljava/util/Map;)V"), eventName2, parameters2);
    else if (!parameters2 && !timed)
		env->CallStaticVoidMethod(cls, env->GetStaticMethodID(cls, "logEvent", "(Ljava/lang/String;)V"), eventName2);

	env->DeleteLocalRef(eventName2);
	
	if (parameters2)
		env->DeleteLocalRef(parameters2);

	env->DeleteLocalRef(cls);
}

void gflurry_EndTimedEvent(const char *eventName, const char **parameters)
{
	JNIEnv *env = g_getJNIEnv();

	jclass cls = env->FindClass("com/giderosmobile/android/plugins/flurry/GFlurry");	

    jstring eventName2 = env->NewStringUTF(eventName);

    jobject parameters2 = parametersToMap(parameters);

	if (parameters2)
		env->CallStaticVoidMethod(cls, env->GetStaticMethodID(cls, "endTimedEvent", "(Ljava/lang/String;Ljava/util/Map;)V"), eventName2, parameters2);
	else
		env->CallStaticVoidMethod(cls, env->GetStaticMethodID(cls, "endTimedEvent", "(Ljava/lang/String;)V"), eventName2);
	
	env->DeleteLocalRef(eventName2);
	
	if (parameters2)
		env->DeleteLocalRef(parameters2);

	env->DeleteLocalRef(cls);
}

}
