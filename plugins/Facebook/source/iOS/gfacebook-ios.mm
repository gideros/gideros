#include "gfacebook.h"
#include "gapplication.h"

#import "Facebook.h"

class GGFacebook;

@interface GGFacebookDelegate : NSObject<FBSessionDelegate, FBDialogDelegate, FBRequestDelegate>
{
}

@property (nonatomic, assign) GGFacebook *facebook;

@end

class GGFacebook
{
public:
    GGFacebook()
    {
		gid_ = g_NextId();

        facebook_ = nil;

        delegate_ = [[GGFacebookDelegate alloc] init];
        delegate_.facebook = this;
        
        gapplication_addCallback(openUrl_s, this);
    }
    
    ~GGFacebook()
    {
        gapplication_removeCallback(openUrl_s, this);
        
        delegate_.facebook = NULL;
        if (facebook_)
            [facebook_ release];
        [delegate_ release];

		gevent_RemoveEventsWithGid(gid_);
    }
    
    void setAppId(const char *appId)
    {
        if (facebook_)
            [facebook_ release];

        facebook_ = [[Facebook alloc] initWithAppId:[NSString stringWithUTF8String:appId] andDelegate:delegate_];
        
        NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
        if ([defaults objectForKey:@"FBAccessTokenKey"] && [defaults objectForKey:@"FBExpirationDateKey"])
        {
            facebook_.accessToken = [defaults objectForKey:@"FBAccessTokenKey"];
            facebook_.expirationDate = [defaults objectForKey:@"FBExpirationDateKey"];
        }
    }

    void authorize(const char * const *permissions)
    {
        NSMutableArray *permissions2 = nil;
        if (permissions)
        {
            permissions2 = [NSMutableArray array];
            while (*permissions)
            {
                [permissions2 addObject:[NSString stringWithUTF8String:*permissions]];
                permissions++;
            }
        }

        [facebook_ authorize:permissions2];
    }
    
    void logout()
    {
        [facebook_ logout:delegate_];
    }
    
    int isSessionValid()
    {
        return facebook_.isSessionValid;
    }
    
    void dialog(const char *action, const gfacebook_Parameter *params)
    {
        NSString *action2 = [NSString stringWithUTF8String:action];

        NSMutableDictionary *params2 = nil;
        if (params)
        {
            params2 = [NSMutableDictionary dictionary];
            while (params->key)
            {
                [params2 setValue:[NSString stringWithUTF8String:params->value] forKey:[NSString stringWithUTF8String:params->key]];
                ++params;
            }
        }

        if (params2)
            [facebook_ dialog:action2 andParams:params2 andDelegate:delegate_];
        else
            [facebook_ dialog:action2 andDelegate:delegate_];
    }

    void graphRequest(const char *graphPath, const gfacebook_Parameter *params, const char *httpMethod)
    {
        NSString *graphPath2 = [NSString stringWithUTF8String:graphPath];

        NSMutableDictionary *params2 = nil;
        if (params)
        {
            params2 = [NSMutableDictionary dictionary];
            while (params->key)
            {
                [params2 setValue:[NSString stringWithUTF8String:params->value] forKey:[NSString stringWithUTF8String:params->key]];
                ++params;
            }
        }
        
        NSString *httpMethod2 = httpMethod ? [NSString stringWithUTF8String:httpMethod] : nil;

        if (params2 && httpMethod2)
        {
            [facebook_ requestWithGraphPath:graphPath2 andParams:params2 andHttpMethod:httpMethod2 andDelegate:delegate_];
        }
        else if (!params2 && httpMethod2)
        {
            params2 = [NSMutableDictionary dictionary];
            [facebook_ requestWithGraphPath:graphPath2 andParams:params2 andHttpMethod:httpMethod2 andDelegate:delegate_];
        }
        else if (params2 && !httpMethod2)
        {
            [facebook_ requestWithGraphPath:graphPath2 andParams:params2 andDelegate:delegate_];
        }
        else if (!params2 && !httpMethod2)
        {
            [facebook_ requestWithGraphPath:graphPath2 andDelegate:delegate_];
        }
    }

    void setAccessToken(const char *accessToken)
    {
        facebook_.accessToken = [NSString stringWithUTF8String:accessToken];
    }
    
    const char *getAccessToken()
    {
        return facebook_.accessToken ? [facebook_.accessToken UTF8String] : NULL;
    }
    
    void setExpirationDate(time_t time)
    {
        facebook_.expirationDate = [NSDate dateWithTimeIntervalSince1970:time];
    }
    
    time_t getExpirationDate()
    {
        return facebook_.expirationDate ? [facebook_.expirationDate timeIntervalSince1970] : -1;
    }

    void extendAccessToken()
    {
        [facebook_ extendAccessToken];
    }
    
    void extendAccessTokenIfNeeded()
    {
        [facebook_ extendAccessTokenIfNeeded];
    }

    int shouldExtendAccessToken()
    {
        return [facebook_ shouldExtendAccessToken];
    }

    void fbDidLogin()
    {
        NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
        [defaults setObject:facebook_.accessToken forKey:@"FBAccessTokenKey"];
        [defaults setObject:facebook_.expirationDate forKey:@"FBExpirationDateKey"];
        [defaults synchronize];

        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_LOGIN_COMPLETE_EVENT, NULL, 0, this);
    }
    
    void fbDidNotLogin(BOOL cancelled)
    {
        if (cancelled)
            gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_LOGIN_CANCEL_EVENT, NULL, 0, this);
        else
            gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_LOGIN_ERROR_EVENT, NULL, 0, this);
    }
        
    void fbDidLogout()
    {
        NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
        [defaults removeObjectForKey:@"FBAccessTokenKey"];
        [defaults removeObjectForKey:@"FBExpirationDateKey"];
        [defaults synchronize];
        
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_LOGOUT_COMPLETE_EVENT, NULL, 0, this);
    }
    
    void dialogDidComplete()
    {
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_DIALOG_COMPLETE_EVENT, NULL, 0, this);
    }
    
    void dialogDidNotComplete()
    {
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_DIALOG_CANCEL_EVENT, NULL, 0, this);
    }
    
    void dialog_didFailWithError(NSError *error)
    {        
        gfacebook_DialogErrorEvent *event = (gfacebook_DialogErrorEvent*)gevent_CreateEventStruct1(
            sizeof(gfacebook_DialogErrorEvent),
            offsetof(gfacebook_DialogErrorEvent, errorDescription), [error.description UTF8String]);
        
        event->errorCode = error.code;
        
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_DIALOG_ERROR_EVENT, event, 1, this);
    }
        
    void request_didFailWithError(NSError *error)
    {
        gfacebook_RequestErrorEvent *event = (gfacebook_RequestErrorEvent*)gevent_CreateEventStruct1(
            sizeof(gfacebook_RequestErrorEvent),
            offsetof(gfacebook_RequestErrorEvent, errorDescription), [error.description UTF8String]);
        
        event->errorCode = error.code;
        
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_REQUEST_ERROR_EVENT, event, 1, this);
    }

    void request_didLoadRawResponse(NSData *data)
    {
        int size = sizeof(gfacebook_RequestCompleteEvent) + data.length + 1;
        gfacebook_RequestCompleteEvent *event = (gfacebook_RequestCompleteEvent*)malloc(size);
        
        event->response = (char*)event + sizeof(gfacebook_RequestCompleteEvent);
        memcpy((char*)event->response, data.bytes, data.length);
        ((char*)event->response)[data.length] = 0;
        event->responseLength = data.length;
        
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
            if (facebook_)
            {
                gapplication_OpenUrlEvent *event2 = (gapplication_OpenUrlEvent*)event;
                [facebook_ handleOpenURL:[NSURL URLWithString:[NSString stringWithUTF8String:event2->url]]];
            }
        }
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
    Facebook *facebook_;
    GGFacebookDelegate *delegate_;

private:
	gevent_CallbackList callbackList_;
	g_id gid_;
};

@implementation GGFacebookDelegate

@synthesize facebook = facebook_;

//FBSessionDelegate
- (void)fbDidLogin
{
    if (facebook_)
        facebook_->fbDidLogin();
}

- (void)fbDidNotLogin:(BOOL)cancelled
{
    if (facebook_)
        facebook_->fbDidNotLogin(cancelled);
}

- (void)fbDidExtendToken:(NSString*)accessToken
               expiresAt:(NSDate*)expiresAt
{
    // no equivalent on Android side
}

- (void)fbDidLogout
{
    if (facebook_)
        facebook_->fbDidLogout();
}

- (void)fbSessionInvalidated
{
    // no equivalent on Android side
}


// FBDialogDelegate
- (void)dialogDidComplete:(FBDialog *)dialog
{
}

- (void)dialogCompleteWithUrl:(NSURL *)url
{
    if (url.query)
    {
        if (facebook_)
            facebook_->dialogDidComplete();
    }
    else
    {
        if (facebook_)
            facebook_->dialogDidNotComplete();        
    }
}

- (void)dialogDidNotCompleteWithUrl:(NSURL *)url
{
    if (facebook_)
        facebook_->dialogDidNotComplete();
}

- (void)dialogDidNotComplete:(FBDialog *)dialog
{
}

- (void)dialog:(FBDialog*)dialog didFailWithError:(NSError *)error
{
    if (facebook_)
        facebook_->dialog_didFailWithError(error);
}


// FBRequestDelegate
- (void)requestLoading:(FBRequest *)request
{
}

- (void)request:(FBRequest *)request didReceiveResponse:(NSURLResponse *)response
{
}

- (void)request:(FBRequest *)request didFailWithError:(NSError *)error
{
    if (facebook_)
        facebook_->request_didFailWithError(error);
}

- (void)request:(FBRequest *)request didLoad:(id)result
{
}

- (void)request:(FBRequest *)request didLoadRawResponse:(NSData *)data
{
    if (facebook_)
        facebook_->request_didLoadRawResponse(data);
}

@end

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
    return s_facebook->setExpirationDate(time);
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
