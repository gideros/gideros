#include <gfacebook.h>
#include <jni.h>
#include <stdlib.h>
#include <glog.h>
#include <string>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

class GGFacebook
{
public:
    GGFacebook()
    {
		gid_ = g_NextId();
	
		JNIEnv *env = g_getJNIEnv();

		jclass localClass = env->FindClass("com/giderosmobile/android/plugins/facebook/GFacebook");
		cls_ = (jclass)env->NewGlobalRef(localClass);
		env->DeleteLocalRef(localClass);
		
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "init", "(J)V"), (jlong)this);
		
		jclass localClass2 = env->FindClass("android/os/Bundle");
		clsBundle_ = (jclass)env->NewGlobalRef(localClass2);
		env->DeleteLocalRef(localClass2);
    }
    
    ~GGFacebook()
    {
        JNIEnv *env = g_getJNIEnv();

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "cleanup", "()V"));
		
		env->DeleteGlobalRef(cls_);
		env->DeleteGlobalRef(clsBundle_);

		gevent_RemoveEventsWithGid(gid_);
    }
    
    void setAppId(const char *appId)
    {
        JNIEnv *env = g_getJNIEnv();
		
		jstring jappId = env->NewStringUTF(appId);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setAppId", "(Ljava/lang/String;)V"), jappId);
		env->DeleteLocalRef(jappId);
    }
	
	void authorize(const char * const *permissions)
    {
		JNIEnv *env = g_getJNIEnv();
		if(permissions)
		{
			int size = 0;
			const char * const *permissions2 = permissions;
			while (*permissions2)
			{
				size++;
				permissions2++;
			}
		
			jobjectArray ret = (jobjectArray)env->NewObjectArray(size,  
				env->FindClass("java/lang/String"),  
				NULL);  
   
			for(int i=0; i<size; i++) {  
				jstring jstr = env->NewStringUTF(*permissions);
				env->SetObjectArrayElement(ret,i,jstr);
				env->DeleteLocalRef(jstr);
				permissions++;
			} 
			
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "authorize", "([Ljava/lang/Object;)V"), ret);

			env->DeleteLocalRef(ret);
		}
		else
		{
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "authorize", "()V"));
		}
    }
    
    void logout()
    {
        JNIEnv *env = g_getJNIEnv();
		
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "logout", "()V"));
    }
    
    int isSessionValid()
    {
		JNIEnv *env = g_getJNIEnv();
		
		int isValid = (int)env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "isSessionValid", "()Z"));
        return isValid;
    }
    
    void dialog(const char *action, const gfacebook_Parameter *params)
    {
		JNIEnv *env = g_getJNIEnv();
		
		jstring jaction = env->NewStringUTF(action);
		
		if(params)
		{
			jobject jbundleobj = env->NewObject(clsBundle_, env->GetMethodID(clsBundle_, "<init>", "()V"));
			while (params->key)
            {
				jstring jKey = env->NewStringUTF(params->key);
				jstring jVal = env->NewStringUTF(params->value);
				env->CallObjectMethod(jbundleobj, env->GetMethodID(clsBundle_, "putString", "(Ljava/lang/String;Ljava/lang/String;)V"), jKey, jVal);
				env->DeleteLocalRef(jKey);
				env->DeleteLocalRef(jVal);
				++params;
			}
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "dialog", "(Ljava/lang/String;Ljava/lang/Object;)V"), jaction, jbundleobj);
			env->DeleteLocalRef(jbundleobj);
		}
		else
		{
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "dialog", "(Ljava/lang/String;)V"), jaction);
		}
		
		env->DeleteLocalRef(jaction);
    }

    void graphRequest(const char *graphPath, const gfacebook_Parameter *params, const char *httpMethod)
    {
		JNIEnv *env = g_getJNIEnv();
		
		jstring jgraphPath = env->NewStringUTF(graphPath);
		
		jobject jbundleobj = NULL;
		if (params)
        {
			jbundleobj = env->NewObject(clsBundle_, env->GetMethodID(clsBundle_, "<init>", "()V"));
			while (params->key)
            {
				jstring jKey = env->NewStringUTF(params->key);
				jstring jVal = env->NewStringUTF(params->value);
				env->CallObjectMethod(jbundleobj, env->GetMethodID(clsBundle_, "putString", "(Ljava/lang/String;Ljava/lang/String;)V"), jKey, jVal);
				env->DeleteLocalRef(jKey);
				env->DeleteLocalRef(jVal);
				++params;
			}
		}
		
		jstring jhttpMethod = NULL;
		if(httpMethod)
		{
			jhttpMethod = env->NewStringUTF(httpMethod);
		}

        if (jbundleobj && jhttpMethod)
        {
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "graphRequest", "(Ljava/lang/String;Ljava/lang/Object;Ljava/lang/String;)V"), jgraphPath, jbundleobj, jhttpMethod);
        }
        else if (!jbundleobj && jhttpMethod)
        {
			jbundleobj = env->NewObject(clsBundle_, env->GetMethodID(clsBundle_, "<init>", "()V"));
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "graphRequest", "(Ljava/lang/String;Ljava/lang/Object;Ljava/lang/String;)V"), jgraphPath, jbundleobj, jhttpMethod);
        }
        else if (jbundleobj && !jhttpMethod)
        {
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "graphRequest", "(Ljava/lang/String;Ljava/lang/Object;)V"), jgraphPath, jbundleobj);
        }
        else if (!jbundleobj && !jhttpMethod)
        {
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "graphRequest", "(Ljava/lang/String;)V"), jgraphPath);
        }
		
		if (jbundleobj)
			env->DeleteLocalRef(jbundleobj);

		if (jhttpMethod)
			env->DeleteLocalRef(jhttpMethod);

		env->DeleteLocalRef(jgraphPath);
    }

    void setAccessToken(const char *accessToken)
    {
         JNIEnv *env = g_getJNIEnv();
		
		jstring jaccessToken = env->NewStringUTF(accessToken);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setAccessToken", "(Ljava/lang/String;)V"), jaccessToken);
		env->DeleteLocalRef(jaccessToken);
    }
    
    const char *getAccessToken()
    {
		JNIEnv *env = g_getJNIEnv();
		
		jstring jtoken = (jstring)env->CallStaticObjectMethod(cls_, env->GetStaticMethodID(cls_, "getAccessToken", "()Ljava/lang/String;"));
       const char *token = env->GetStringUTFChars(jtoken, NULL);
	   accessToken_ = token;
	   env->ReleaseStringUTFChars(jtoken, token);
	   
	   return accessToken_.c_str();
    }
    
    void setExpirationDate(time_t time)
    {
		JNIEnv *env = g_getJNIEnv();

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setExpirationDate", "(J)V"), (jlong)time);
    }
    
    time_t getExpirationDate()
    {
		JNIEnv *env = g_getJNIEnv();
		time_t time = (time_t)env->CallStaticLongMethod(cls_, env->GetStaticMethodID(cls_, "getExpirationDate", "()J"));
        return time;
    }

    void extendAccessToken()
    {
        JNIEnv *env = g_getJNIEnv();
		
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "extendAccessToken", "()V"));
    }
    
    void extendAccessTokenIfNeeded()
    {
        JNIEnv *env = g_getJNIEnv();
		
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "extendAccessTokenIfNeeded", "()V"));
    }

    int shouldExtendAccessToken()
    {
		JNIEnv *env = g_getJNIEnv();
		
		int should = (int)env->CallStaticBooleanMethod(cls_, env->GetStaticMethodID(cls_, "shouldExtendAccessToken", "()Z"));
        return should;
    }
	
    void onLoginComplete()
    {
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_LOGIN_COMPLETE_EVENT, NULL, 0, this);
    }
	
	void onLoginError()
    {
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_LOGIN_ERROR_EVENT, NULL, 0, this);
    }
    
	void onLoginCancel()
    {
       gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_LOGIN_CANCEL_EVENT, NULL, 0, this);
    }
	
	void onLogoutComplete()
    {
       gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_LOGOUT_COMPLETE_EVENT, NULL, 0, this);
    }
	
	void onDialogComplete()
    {
       gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_DIALOG_COMPLETE_EVENT, NULL, 0, this);
    }
    
	void onDialogCancel()
    {
       gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_DIALOG_CANCEL_EVENT, NULL, 0, this);
    }
	
	void onDialogError(jint jerrorCode, jstring jerrorDescr)
    {
		JNIEnv *env = g_getJNIEnv();

		const char *error = env->GetStringUTFChars(jerrorDescr, NULL);
		
        gfacebook_DialogErrorEvent *event = (gfacebook_DialogErrorEvent*)gevent_CreateEventStruct1(
            sizeof(gfacebook_DialogErrorEvent),
            offsetof(gfacebook_DialogErrorEvent, errorDescription), error);
        
        event->errorCode = (int)jerrorCode;
        
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_DIALOG_ERROR_EVENT, event, 1, this);
		
		env->ReleaseStringUTFChars(jerrorDescr, error);
    }
	
	void onRequestError(jint jerrorCode, jstring jerrorDescr)
    {
		JNIEnv *env = g_getJNIEnv();

		const char *error = env->GetStringUTFChars(jerrorDescr, NULL);
		
        gfacebook_RequestErrorEvent *event = (gfacebook_RequestErrorEvent*)gevent_CreateEventStruct1(
            sizeof(gfacebook_RequestErrorEvent),
            offsetof(gfacebook_RequestErrorEvent, errorDescription), error);
        
        event->errorCode = (int)jerrorCode;
        
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_REQUEST_ERROR_EVENT, event, 1, this);
		
		env->ReleaseStringUTFChars(jerrorDescr, error);
    }
	
	void onRequestComplete(jstring jresponse)
    {
		JNIEnv *env = g_getJNIEnv();

		const char *response = env->GetStringUTFChars(jresponse, NULL);
		
        gfacebook_RequestCompleteEvent *event = (gfacebook_RequestCompleteEvent*)gevent_CreateEventStruct1(
            sizeof(gfacebook_RequestCompleteEvent),
            offsetof(gfacebook_RequestCompleteEvent, response), response);
        
		event->responseLength = strlen(response);
		
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_REQUEST_COMPLETE_EVENT, event, 1, this);
		
		env->ReleaseStringUTFChars(jresponse, response);
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
		((GGFacebook*)udata)->callback(type, event);
	}

	void callback(int type, void *event)
	{
		callbackList_.dispatchEvent(type, event);
	}

private:
	gevent_CallbackList callbackList_;

private:
	jclass cls_;
	jclass clsBundle_;
	g_id gid_;
	std::string accessToken_;
};


extern "C" {

void Java_com_giderosmobile_android_plugins_facebook_GFacebook_onLoginComplete(JNIEnv *env, jclass clz, jlong data)
{
	((GGFacebook*)data)->onLoginComplete();
}

void Java_com_giderosmobile_android_plugins_facebook_GFacebook_onLoginError(JNIEnv *env, jclass clz, jlong data)
{
	((GGFacebook*)data)->onLoginError();
}

void Java_com_giderosmobile_android_plugins_facebook_GFacebook_onLoginCancel(JNIEnv *env, jclass clz, jlong data)
{
	((GGFacebook*)data)->onLoginCancel();
}

void Java_com_giderosmobile_android_plugins_facebook_GFacebook_onLogoutComplete(JNIEnv *env, jclass clz, jlong data)
{
	((GGFacebook*)data)->onLogoutComplete();
}

void Java_com_giderosmobile_android_plugins_facebook_GFacebook_onDialogComplete(JNIEnv *env, jclass clz, jlong data)
{
	((GGFacebook*)data)->onDialogComplete();
}

void Java_com_giderosmobile_android_plugins_facebook_GFacebook_onDialogError(JNIEnv *env, jclass clz, jint jerrorCode, jstring jerrorDescr, jlong data)
{
	((GGFacebook*)data)->onDialogError(jerrorCode, jerrorDescr);
}

void Java_com_giderosmobile_android_plugins_facebook_GFacebook_onDialogCancel(JNIEnv *env, jclass clz, jlong data)
{
	((GGFacebook*)data)->onDialogCancel();
}

void Java_com_giderosmobile_android_plugins_facebook_GFacebook_onRequestComplete(JNIEnv *env, jclass clz, jstring jresponse, jlong data)
{
	((GGFacebook*)data)->onRequestComplete(jresponse);
}

void Java_com_giderosmobile_android_plugins_facebook_GFacebook_onRequestError(JNIEnv *env, jclass clz, jint jerrorCode, jstring jerrorDescr, jlong data)
{
	((GGFacebook*)data)->onRequestError(jerrorCode, jerrorDescr);
}

}

static GGFacebook *s_facebook = NULL;

extern "C" {

int gfacebook_isAvailable()
{
    return 1;
}

void gfacebook_init()
{
    s_facebook = new GGFacebook;
}

void gfacebook_cleanup()
{
    delete s_facebook;
    s_facebook = NULL;
}

void gfacebook_setAppId(const char *appId)
{
    s_facebook->setAppId(appId);
}

void gfacebook_authorize(const char * const *permissions)
{
    s_facebook->authorize(permissions);
}

void gfacebook_logout()
{
    s_facebook->logout();
}

int gfacebook_isSessionValid()
{
    return s_facebook->isSessionValid();
}
    
void gfacebook_dialog(const char *action, const gfacebook_Parameter *params)
{
    s_facebook->dialog(action, params);
}

void gfacebook_graphRequest(const char *graphPath, const gfacebook_Parameter *params, const char *httpMethod)
{
    s_facebook->graphRequest(graphPath, params, httpMethod);
}

void gfacebook_setAccessToken(const char *accessToken)
{
    s_facebook->setAccessToken(accessToken);
}

const char *gfacebook_getAccessToken()
{
    return s_facebook->getAccessToken();
}

void gfacebook_setExpirationDate(time_t time)
{
    s_facebook->setExpirationDate(time);
}

time_t gfacebook_getExpirationDate()
{
    return s_facebook->getExpirationDate();
}
    
void gfacebook_extendAccessToken()
{
	s_facebook->extendAccessToken();
}

void gfacebook_extendAccessTokenIfNeeded()
{
    s_facebook->extendAccessTokenIfNeeded();
}
    
int gfacebook_shouldExtendAccessToken()
{
    return s_facebook->shouldExtendAccessToken();
}

g_id gfacebook_addCallback(gevent_Callback callback, void *udata)
{
	return s_facebook->addCallback(callback, udata);
}

void gfacebook_removeCallback(gevent_Callback callback, void *udata)
{
	s_facebook->removeCallback(callback, udata);
}

void gfacebook_removeCallbackWithGid(g_id gid)
{
	s_facebook->removeCallbackWithGid(gid);
}

}
