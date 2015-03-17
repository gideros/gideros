#include <gfacebook.h>
#include <jni.h>
#include <stdlib.h>
#include <glog.h>
#include <string>
#include <gapplication.h>

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
		
		gapplication_addCallback(openUrl_s, this);
    }
    
    ~GGFacebook()
    {
		 gapplication_removeCallback(openUrl_s, this);
        JNIEnv *env = g_getJNIEnv();

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "cleanup", "()V"));
		
		env->DeleteGlobalRef(cls_);
		env->DeleteGlobalRef(clsBundle_);

		gevent_RemoveEventsWithGid(gid_);
    }
	
	void login(const char *appId, const char * const *permissions)
    {
		JNIEnv *env = g_getJNIEnv();
		jstring jappId = env->NewStringUTF(appId);
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
			
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "login", "(Ljava/lang/String;[Ljava/lang/Object;)V"), jappId, ret);

			env->DeleteLocalRef(ret);
		}
		else
		{
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "login", "(Ljava/lang/String;)V"), jappId);
		}
		env->DeleteLocalRef(jappId);
    }
    
    void logout()
    {
        JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "logout", "()V"));
    }
	
	const char* getAccessToken(){
		JNIEnv *env = g_getJNIEnv();
		
		jstring jtoken = (jstring)env->CallStaticObjectMethod(cls_, env->GetStaticMethodID(cls_, "getAccessToken", "()Ljava/lang/String;"));
		const char *token = env->GetStringUTFChars(jtoken, NULL);
		accessToken_ = token;
		env->ReleaseStringUTFChars(jtoken, token);
	   
		return accessToken_.c_str();
    }
    
    time_t getExpirationDate(){
        JNIEnv *env = g_getJNIEnv();
		time_t time = (time_t)env->CallStaticLongMethod(cls_, env->GetStaticMethodID(cls_, "getExpirationDate", "()J"));
        return time;
    }
	
	void upload(const char *path, const char *orig)
    {
		JNIEnv *env = g_getJNIEnv();
		
		jstring jpath = env->NewStringUTF(path);
		jstring jorig = env->NewStringUTF(orig);
		
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "upload", "(Ljava/lang/String;Ljava/lang/String;)V"), jpath, jorig);
		
		env->DeleteLocalRef(jpath);
		env->DeleteLocalRef(jorig);
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

    void request(const char *graphPath, const gfacebook_Parameter *params, int httpMethod)
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

        if (jbundleobj)
        {
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "request", "(Ljava/lang/String;ILjava/lang/Object;)V"), jgraphPath, (jint)httpMethod, jbundleobj);
        }
        else
        {
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "request", "(Ljava/lang/String;I)V"), jgraphPath, (jint)httpMethod);
        }
		
		if (jbundleobj)
			env->DeleteLocalRef(jbundleobj);
			
		env->DeleteLocalRef(jgraphPath);
    }
	
    void onLoginComplete()
    {
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_LOGIN_COMPLETE_EVENT, NULL, 0, this);
    }
	
	void onLoginError(jstring jretValue)
    {
		JNIEnv *env = g_getJNIEnv();

		const char *value = env->GetStringUTFChars(jretValue, NULL);
		
        gfacebook_SimpleEvent *event = (gfacebook_SimpleEvent*)gevent_CreateEventStruct1(
            sizeof(gfacebook_SimpleEvent),
            offsetof(gfacebook_SimpleEvent, value), value);
        
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_LOGIN_ERROR_EVENT, event, 1, this);
		
		env->ReleaseStringUTFChars(jretValue, value);
    }
	
	void onLogoutComplete()
    {
       gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_LOGOUT_COMPLETE_EVENT, NULL, 0, this);
    }
	
	void onLogoutError(jstring jretValue)
    {
		JNIEnv *env = g_getJNIEnv();

		const char *value = env->GetStringUTFChars(jretValue, NULL);
		
        gfacebook_SimpleEvent *event = (gfacebook_SimpleEvent*)gevent_CreateEventStruct1(
            sizeof(gfacebook_SimpleEvent),
            offsetof(gfacebook_SimpleEvent, value), value);
        
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_LOGOUT_ERROR_EVENT, event, 1, this);
		
		env->ReleaseStringUTFChars(jretValue, value);
    }
	
	void onOpenUrl(jstring jretValue)
    {
		JNIEnv *env = g_getJNIEnv();

		const char *value = env->GetStringUTFChars(jretValue, NULL);
		
        gfacebook_SimpleEvent *event = (gfacebook_SimpleEvent*)gevent_CreateEventStruct1(
            sizeof(gfacebook_SimpleEvent),
            offsetof(gfacebook_SimpleEvent, value), value);
        
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_OPEN_URL_EVENT, event, 1, this);
		
		env->ReleaseStringUTFChars(jretValue, value);
    }
	
	void onDialogComplete(jstring jretType, jstring jretValue)
    {
       JNIEnv *env = g_getJNIEnv();

		const char *type = env->GetStringUTFChars(jretType, NULL);
		const char *value = env->GetStringUTFChars(jretValue, NULL);
		
        gfacebook_DoubleEvent *event = (gfacebook_DoubleEvent*)gevent_CreateEventStruct2(
			sizeof(gfacebook_DoubleEvent),
			offsetof(gfacebook_DoubleEvent, type), type,
			offsetof(gfacebook_DoubleEvent, value), value);
        
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_DIALOG_COMPLETE_EVENT, event, 1, this);
		
		env->ReleaseStringUTFChars(jretType, type);
		env->ReleaseStringUTFChars(jretValue, value);
    }
	
	void onDialogError(jstring jretType, jstring jretValue)
    {
		JNIEnv *env = g_getJNIEnv();

		const char *type = env->GetStringUTFChars(jretType, NULL);
		const char *value = env->GetStringUTFChars(jretValue, NULL);
		
        gfacebook_DoubleEvent *event = (gfacebook_DoubleEvent*)gevent_CreateEventStruct2(
			sizeof(gfacebook_DoubleEvent),
			offsetof(gfacebook_DoubleEvent, type), type,
			offsetof(gfacebook_DoubleEvent, value), value);
        
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_DIALOG_ERROR_EVENT, event, 1, this);
		
		env->ReleaseStringUTFChars(jretType, type);
		env->ReleaseStringUTFChars(jretValue, value);
    }
	
	void onRequestError(jstring jretType, jstring jretValue)
    {
		JNIEnv *env = g_getJNIEnv();

		const char *type = env->GetStringUTFChars(jretType, NULL);
		const char *value = env->GetStringUTFChars(jretValue, NULL);
		
        gfacebook_DoubleEvent *event = (gfacebook_DoubleEvent*)gevent_CreateEventStruct2(
			sizeof(gfacebook_DoubleEvent),
			offsetof(gfacebook_DoubleEvent, type), type,
			offsetof(gfacebook_DoubleEvent, value), value);
        
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_REQUEST_ERROR_EVENT, event, 1, this);
		
		env->ReleaseStringUTFChars(jretType, type);
		env->ReleaseStringUTFChars(jretValue, value);
    }
	
	void onRequestComplete(jstring jretType, jstring jresponse)
    {
		JNIEnv *env = g_getJNIEnv();
		
		const char *type = env->GetStringUTFChars(jretType, NULL);
		const char *response = env->GetStringUTFChars(jresponse, NULL);
		
		gfacebook_ResponseEvent *event = (gfacebook_ResponseEvent*)gevent_CreateEventStruct2(
			sizeof(gfacebook_ResponseEvent),
			offsetof(gfacebook_ResponseEvent, type), type,
			offsetof(gfacebook_ResponseEvent, response), response);
        
		event->responseLength = strlen(response);
		
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_REQUEST_COMPLETE_EVENT, event, 1, this);
		
		env->ReleaseStringUTFChars(jretType, type);
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
	static void openUrl_s(int type, void *event, void *udata)
    {
        static_cast<GGFacebook*>(udata)->openUrl(type, event);
    }
    
    void openUrl(int type, void *event)
    {
        if (type == GAPPLICATION_OPEN_URL_EVENT)
        {
            gapplication_OpenUrlEvent *event2 = (gapplication_OpenUrlEvent*)event;
                
            const char* url = event2->url;
                
            gfacebook_SimpleEvent *event3 = (gfacebook_SimpleEvent*)gevent_CreateEventStruct1(
                sizeof(gfacebook_SimpleEvent),
                offsetof(gfacebook_SimpleEvent, value), url);
               
            gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_OPEN_URL_EVENT, event3, 1, this);          
        }
    }
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

void Java_com_giderosmobile_android_plugins_facebook_GFacebook_onLoginError(JNIEnv *env, jclass clz, jstring error, jlong data)
{
	((GGFacebook*)data)->onLoginError(error);
}

void Java_com_giderosmobile_android_plugins_facebook_GFacebook_onLogoutComplete(JNIEnv *env, jclass clz, jlong data)
{
	((GGFacebook*)data)->onLogoutComplete();
}

void Java_com_giderosmobile_android_plugins_facebook_GFacebook_onLogoutError(JNIEnv *env, jclass clz, jstring error, jlong data)
{
	((GGFacebook*)data)->onLogoutError(error);
}

void Java_com_giderosmobile_android_plugins_facebook_GFacebook_onOpenUrl(JNIEnv *env, jclass clz, jstring url, jlong data)
{
	((GGFacebook*)data)->onOpenUrl(url);
}

void Java_com_giderosmobile_android_plugins_facebook_GFacebook_onDialogComplete(JNIEnv *env, jclass clz, jstring type, jstring response, jlong data)
{
	((GGFacebook*)data)->onDialogComplete(type, response);
}

void Java_com_giderosmobile_android_plugins_facebook_GFacebook_onDialogError(JNIEnv *env, jclass clz, jstring type, jstring jerrorDescr, jlong data)
{
	((GGFacebook*)data)->onDialogError(type, jerrorDescr);
}

void Java_com_giderosmobile_android_plugins_facebook_GFacebook_onRequestComplete(JNIEnv *env, jclass clz, jstring type, jstring jresponse, jlong data)
{
	((GGFacebook*)data)->onRequestComplete(type, jresponse);
}

void Java_com_giderosmobile_android_plugins_facebook_GFacebook_onRequestError(JNIEnv *env, jclass clz, jstring type, jstring jerrorDescr, jlong data)
{
	((GGFacebook*)data)->onRequestError(type, jerrorDescr);
}

}

static GGFacebook *s_facebook = NULL;

extern "C" {

void gfacebook_init()
{
    s_facebook = new GGFacebook;
}

void gfacebook_cleanup()
{
    delete s_facebook;
    s_facebook = NULL;
}

void gfacebook_login(const char *appId, const char * const *permissions)
{
    s_facebook->login(appId, permissions);
}

void gfacebook_logout()
{
    s_facebook->logout();
}

void gfacebook_upload(const char *path, const char *orig)
{
    s_facebook->upload(path, orig);
}

const char* gfacebook_getAccessToken(){
    return s_facebook->getAccessToken();
}
    
time_t gfacebook_getExpirationDate(){
    return s_facebook->getExpirationDate();
}
    
void gfacebook_dialog(const char *action, const gfacebook_Parameter *params)
{
    s_facebook->dialog(action, params);
}

void gfacebook_request(const char *graphPath, const gfacebook_Parameter *params, int httpMethod)
{
    s_facebook->request(graphPath, params, httpMethod);
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
