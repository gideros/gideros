#include <jni.h>
#include <glog.h>
#include "gstoreReview.h"

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

bool gstorereview_requestReview()
{
	JNIEnv *env = g_getJNIEnv();

	jclass localClass = env->FindClass("com/giderosmobile/android/plugins/storereview/StoreReview");
	jclass cls_ = (jclass)env->NewGlobalRef(localClass);
	env->DeleteLocalRef(localClass);
	jboolean result = env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "review", "()Z"));
	env->DeleteGlobalRef(cls_);
	return result;
}

