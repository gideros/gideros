#include <gapplication.h>
#include <gapplication-ios.h>
#import <UIKit/UIKit.h>

extern NSString* getSysInfoByName(const char* typeSpecifier);

class GGApplicationManager
{
    
public:
    GGApplicationManager()
    {
        gid_ = g_NextId();
    }
    
    ~GGApplicationManager()
    {
        gevent_RemoveEventsWithGid(gid_);
    }
    
    int getScreenDensity()
    {
        UIViewController *rootViewController = [[UIApplication sharedApplication].delegate viewController];
        UIView *glView = [rootViewController glView];

        float scale = 1;
        if ([glView respondsToSelector:@selector(contentScaleFactor)])
            scale = glView.contentScaleFactor;
            
         NSDictionary *modelDpi = @{
            @"iPhone7,1": @133, //6+ x3=401
            @"iPhone8,2": @133, //6S+
            @"iPhone9,2": @133, //7+
            @"iPhone9,4": @133, //7+
            @"iPad2,5": @163, //Mini
            @"iPad2,6": @163, //Mini
            @"iPad2,7": @163, //Mini
            @"iPad4,4": @163, //Mini2 x2=326
            @"iPad4,5": @163, //Mini2
            @"iPad4,6": @163, //Mini2
            @"iPad4,7": @163, //Mini3
            @"iPad4,8": @163, //Mini3
            @"iPad4,9": @163, //Mini3
            @"iPad5,1": @163, //Mini4
            @"iPad5,2": @163, //Mini4
		};

		NSInteger *mdpi=modelDpi[getSysInfoByName("hw.machine")];
		if (mdpi)
			return [mdpi integerValue]*scale;
		
        int dpi;
        if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
        {
            dpi = 132 * scale;
        }
        else if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone)
        {
            dpi = 163 * scale;
        }
        else
        {
            dpi = 160 * scale;
        }
        
        return dpi;
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
    
    static void callback_s(int type, void *event, void *udata)
    {
        static_cast<GGApplicationManager*>(udata)->callbackList_.dispatchEvent(type, event);
    }
    
    void enqueueEvent(int type, void *event, int free)
    {
        gevent_EnqueueEvent(gid_, callback_s, type, event, free, this);
    }
    
private:
    gevent_CallbackList callbackList_;
    g_id gid_;
};

static GGApplicationManager *s_manager = NULL;

extern "C" {

void gapplication_init()
{
    s_manager = new GGApplicationManager;
}

void gapplication_cleanup()
{
    delete s_manager;
    s_manager = NULL;
}

g_id gapplication_addCallback(gevent_Callback callback, void *udata)
{
    return s_manager->addCallback(callback, udata);
}

void gapplication_removeCallback(gevent_Callback callback, void *udata)
{
    s_manager->removeCallback(callback, udata);
}

void gapplication_removeCallbackWithGid(g_id gid)
{
    s_manager->removeCallbackWithGid(gid);
}

int gapplication_getScreenDensity()
{
    return s_manager->getScreenDensity();
}

void gapplication_exit()
{
        
}

void gapplication_enqueueEvent(int type, void *event, int free)
{
    s_manager->enqueueEvent(type, event, free);
}

}
