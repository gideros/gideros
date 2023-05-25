#include <gapplication.h>
#include <gapplication-ios.h>
#if !TARGET_OS_OSX
#import <UIKit/UIKit.h>
#else
#import <Cocoa/Cocoa.h>
#endif
extern NSString* getSysInfoByName(const char* typeSpecifier);
extern float gdr_ScaleFactor;

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
#if TARGET_OS_OSX
        NSScreen *screen = [NSScreen mainScreen];
        NSDictionary *description = [screen deviceDescription];
        NSSize displayPixelSize = [[description objectForKey:NSDeviceSize] sizeValue];
        CGSize displayPhysicalSize = CGDisplayScreenSize(
                                                         [[description objectForKey:@"NSScreenNumber"] unsignedIntValue]);
        
        float fdpi=(displayPixelSize.width / displayPhysicalSize.width) * 25.4f;
        fdpi=fdpi*[screen backingScaleFactor];
        int dpi=fdpi;
#else
        float scale = gdr_ScaleFactor;
            
         NSDictionary *modelDpi = @{
            @"iPhone7,1": @133, //6+ x3=401
            @"iPhone8,2": @133, //6S+
            @"iPhone9,2": @133, //7+
            @"iPhone9,4": @133, //7+
            @"iPhone10,2": @133, //8+            
            @"iPhone10,5": @133, //8+            
            @"iPhone10,3": @153, //X x3=458            
            @"iPhone10,6": @153, //X           
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

        NSString *model=getSysInfoByName("hw.machine");
		id mdpi=modelDpi[model];
		if (mdpi)
			return [mdpi integerValue]*scale;
        if ([model hasPrefix:@"iPhone"]) {
            NSArray<NSString *>* ca=[[model substringFromIndex:6] componentsSeparatedByString:@","];
            NSString *major=[ca firstObject];
            if (major) {
                int mm=[major intValue];
                if (mm>10) return 153*scale;
            }
        }
		
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
#endif
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

int gapplication_getScreenDensity(int *ldpi)
{
    return s_manager->getScreenDensity();
}

void gapplication_exit()
{
#if TARGET_OS_OSX
    [[NSApplication sharedApplication] terminate:nil];
#endif
}

void gapplication_enqueueEvent(int type, void *event, int free)
{
    s_manager->enqueueEvent(type, event, free);
}

}
