#include "gfacebook.h"
#include <stdlib.h>
#include <glog.h>
#include <string>
#include <gapplication.h>
#import "GFBook.h"

class GGFacebook
{
public:
    GGFacebook()
    {
		gid_ = g_NextId();
        fb = [[GFBook alloc] init];
        gapplication_addCallback(openUrl_s, this);
    }
    
    ~GGFacebook()
    {
        gapplication_removeCallback(openUrl_s, this);
        [fb deinit];
        [fb release];
        fb = nil;
		gevent_RemoveEventsWithGid(gid_);
    }
	
	void login(const char *appId, const char * const *permissions)
    {
        NSMutableArray *arr = [NSMutableArray array];
        if(permissions)
		{
            while (*permissions)
            {
                [arr addObject:[NSString stringWithUTF8String:*permissions]];
                permissions++;
            }
        }
        [fb login:arr];
    }
    
    void logout()
    {
        [fb logout];
    }
    
    void upload(const char *path, const char *orig)
    {
        [fb upload:[NSString stringWithUTF8String:path] with:[NSString stringWithUTF8String:orig]];
    }
    
    const char* getAccessToken(){
        return [[fb getAccessToken] UTF8String];
    }
    
    time_t getExpirationDate(){
        return [fb getExpirationDate];
    }
    
    void dialog(const char *action, const gfacebook_Parameter *params)
    {
		NSMutableDictionary *dic = [NSMutableDictionary dictionary];
        if(params)
		{
            while (params->key)
            {
                [dic setObject:[NSString stringWithUTF8String:params->value] forKey:[NSString stringWithUTF8String:params->key]];
                params++;
            }
        }
        [fb dialog:[NSString stringWithUTF8String:action] withParams:dic];
    }

    void request(const char *graphPath, const gfacebook_Parameter *params, int httpMethod)
    {
        NSMutableDictionary *dic = [NSMutableDictionary dictionary];
        if(params)
		{
            while (params->key)
            {
                [dic setObject:[NSString stringWithUTF8String:params->value] forKey:[NSString stringWithUTF8String:params->key]];
                params++;
            }
        }
        [fb request:[NSString stringWithUTF8String:graphPath] forMethod:httpMethod withParams:dic];
    }
	
    void onLoginComplete()
    {
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_LOGIN_COMPLETE_EVENT, NULL, 0, this);
    }
	
	void onLoginError(const char* value)
    {
		
        gfacebook_SimpleEvent *event = (gfacebook_SimpleEvent*)gevent_CreateEventStruct1(
            sizeof(gfacebook_SimpleEvent),
            offsetof(gfacebook_SimpleEvent, value), value);
        
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_LOGIN_ERROR_EVENT, event, 1, this);
    }
	
	void onLogoutComplete()
    {
       gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_LOGOUT_COMPLETE_EVENT, NULL, 0, this);
    }
	
	void onLogoutError(const char* value)
    {
        gfacebook_SimpleEvent *event = (gfacebook_SimpleEvent*)gevent_CreateEventStruct1(
            sizeof(gfacebook_SimpleEvent),
            offsetof(gfacebook_SimpleEvent, value), value);
        
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_LOGOUT_ERROR_EVENT, event, 1, this);
    }
	
	void onDialogComplete(const char* type, const char* value)
    {
        gfacebook_DoubleEvent *event = (gfacebook_DoubleEvent*)gevent_CreateEventStruct2(sizeof(gfacebook_DoubleEvent),
            offsetof(gfacebook_DoubleEvent, type), type,
            offsetof(gfacebook_DoubleEvent, value), value);
        
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_DIALOG_COMPLETE_EVENT, event, 1, this);
    }
	
	void onDialogError(const char* type, const char* value)
    {
        gfacebook_DoubleEvent *event = (gfacebook_DoubleEvent*)gevent_CreateEventStruct2(
			sizeof(gfacebook_DoubleEvent),
			offsetof(gfacebook_DoubleEvent, type), type,
			offsetof(gfacebook_DoubleEvent, value), value);
        
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_DIALOG_ERROR_EVENT, event, 1, this);
    }
	
	void onRequestError(const char* type, const char* value)
    {
        gfacebook_DoubleEvent *event = (gfacebook_DoubleEvent*)gevent_CreateEventStruct2(
			sizeof(gfacebook_DoubleEvent),
			offsetof(gfacebook_DoubleEvent, type), type,
			offsetof(gfacebook_DoubleEvent, value), value);
        
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_REQUEST_ERROR_EVENT, event, 1, this);

    }
	
	void onRequestComplete(const char* type, const char* response)
    {
		gfacebook_ResponseEvent *event = (gfacebook_ResponseEvent*)gevent_CreateEventStruct2(
			sizeof(gfacebook_ResponseEvent),
			offsetof(gfacebook_ResponseEvent, type), type,
			offsetof(gfacebook_ResponseEvent, response), response);
        
		event->responseLength = strlen(response);
		
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_REQUEST_COMPLETE_EVENT, event, 1, this);
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
            if(fb != NULL)
            {
                gapplication_OpenUrlEvent *event2 = (gapplication_OpenUrlEvent*)event;
                
                const char* url = event2->url;
                
                [fb handleOpenUrl:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
                
                gfacebook_SimpleEvent *event3 = (gfacebook_SimpleEvent*)gevent_CreateEventStruct1(
                    sizeof(gfacebook_SimpleEvent),
                    offsetof(gfacebook_SimpleEvent, value), url);
                
                gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_OPEN_URL_EVENT, event3, 1, this);
            }
            
        }
        else if(type == GAPPLICATION_RESUME_EVENT){
            if(fb != NULL)
            {
                [fb applicationDidBecomeActive];
            }
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
	g_id gid_;
	std::string accessToken_;
    GFBook *fb;
};

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
    
void gfacebook_upload(const char *path, const char *orig)
{
   s_facebook->upload(path, orig);
}
    
void gfacebook_onLoginComplete(){
    s_facebook->onLoginComplete();
}
void gfacebook_onLoginError(const char* value){
    s_facebook->onLoginError(value);
}
void gfacebook_onLogoutComplete(){
    s_facebook->onLogoutComplete();
}
void gfacebook_onLogoutError(const char* value){
    s_facebook->onLogoutError(value);
}
void gfacebook_onDialogComplete(const char* type, const char* value){
    s_facebook->onDialogComplete(type,value);
}
void gfacebook_onDialogError(const char* type, const char* value){
    s_facebook->onDialogError(type, value);
}
void gfacebook_onRequestError(const char* type, const char* value){
    s_facebook->onRequestError(type, value);
}
void gfacebook_onRequestComplete(const char* type, const char* response){
    s_facebook->onRequestComplete(type, response);
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
