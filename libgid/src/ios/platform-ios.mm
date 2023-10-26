#if TARGET_OS_OSX
#import <Cocoa/Cocoa.h>
#define UIApplication NSApplication
#define UIViewController NSViewController
#else
#import <UIKit/UIKit.h>
#endif
#import <AudioToolbox/AudioToolbox.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

#include <vector>
#include <string>

#include <sys/socket.h> // Per msqr
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_dl.h>
#include "platform.h"

// typeSpecifier -> "hw.machine" or "hw.model"
NSString* getSysInfoByName(const char* typeSpecifier)
{
    size_t size;
    sysctlbyname(typeSpecifier, NULL, &size, NULL, 0);
    
    char* answer = (char*)malloc(size);
    sysctlbyname(typeSpecifier, answer, &size, NULL, 0);
    
    NSString* results = [NSString stringWithCString:answer encoding: NSUTF8StringEncoding];
	
    free(answer);
    return results;
}


std::vector<std::string> getDeviceInfo()
{
	std::vector<std::string> result;

#if TARGET_OS_OSX
    result.push_back("MacOS");
    result.push_back([[[NSProcessInfo processInfo] operatingSystemVersionString] UTF8String]);
    result.push_back([getSysInfoByName("hw.model") UTF8String]);
#else
	UIDevice* device = [UIDevice currentDevice];

	result.push_back("iOS");
	result.push_back([[device systemVersion] UTF8String]);
	result.push_back([[device model] UTF8String]);
	switch (UI_USER_INTERFACE_IDIOM())
	{
		case UIUserInterfaceIdiomPhone:
			result.push_back("iPhone");
			break;
		case UIUserInterfaceIdiomPad:
			result.push_back("iPad");
			break;
#if TARGET_OS_TV == 1
		case UIUserInterfaceIdiomTV:
			result.push_back("AppleTV");
			break;
#endif
		default:
			result.push_back("");
	}	
#endif
    result.push_back([getSysInfoByName("hw.machine") UTF8String]);
	
	return result;
}

void setWindowSize(int width, int height){
#if TARGET_OS_OSX
   NSWindow *win=[[[NSApplication sharedApplication] delegate] window];
   [win setContentSize:NSMakeSize(width, height)];
#endif
}

void setFullScreen(bool fullScreen){
#if TARGET_OS_OSX
    NSWindow *win=[[[NSApplication sharedApplication] delegate] window];
    bool isFs=[win styleMask]&NSWindowStyleMaskFullScreen;
    if (isFs!=fullScreen)
        [win toggleFullScreen:nil];
#endif
}

std::string getDeviceName(){
#if TARGET_OS_OSX
    return [[[NSHost currentHost] localizedName] UTF8String];
#else
    return [[[UIDevice currentDevice] name] UTF8String];
#endif
}

void vibrate(int ms)
{
#if TARGET_OS_OSX
#else
	AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
#endif
}

void openUrl(const char* url)
{
#if TARGET_OS_OSX
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
#else
	[[UIApplication sharedApplication] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
#endif
}

bool canOpenUrl(const char *url)
{
#if TARGET_OS_OSX
    return true;
    //[[NSWorkspace sharedWorkspace] canOpenURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
#else
	return [[UIApplication sharedApplication] canOpenURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
#endif
}

std::string getLocale()
{
	NSString* locale = [[NSLocale currentLocale] localeIdentifier];
		
	return [locale UTF8String];
}

std::string getLanguage()
{
	NSString* language = [[NSLocale preferredLanguages] objectAtIndex:0];
	
	return [language UTF8String];
}

void setKeepAwake(bool awake)
{
#if TARGET_OS_OSX
#else
	[[UIApplication sharedApplication] setIdleTimerDisabled:(awake ? YES : NO)];
#endif
}

std::string getAppId(){
	return [[[NSBundle mainBundle] bundleIdentifier] UTF8String];
}

extern "C" {
UIViewController *g_getRootViewController();
}

static int s_fps = 60;

extern "C" {

int g_getFps()
{
    return s_fps;
}

void g_setFps(int fps)
{
	if (fps != s_fps)
	{
		UIViewController *viewController = g_getRootViewController();
		switch (fps)
		{
		case 30:
			[viewController setAnimationFrameInterval:2];
			break;
		case 60:
			[viewController setAnimationFrameInterval:1];
			break;
		}
		s_fps = fps;
	}
}

}

void g_exit()
{
    exit(0);
}

std::vector<gapplication_Variant> g_getsetProperty(bool set, const char* what, std::vector<gapplication_Variant> &args)
{
	std::vector<gapplication_Variant> rets;
	gapplication_Variant r;
	if (!set) {
#if TARGET_OS_IOS
		if (!strcmp(what,"batteryLevel"))
		{
			r.type=gapplication_Variant::STRING;
			UIDevice *myDevice = [UIDevice currentDevice];    
			[myDevice setBatteryMonitoringEnabled:YES];
			double batLeft = (float)[myDevice batteryLevel] * 100;
			NSString * levelLabel = [NSString stringWithFormat:@"%.f", batLeft];
			r.s=[levelLabel UTF8String];
			rets.push_back(r);
		}
#elif TARGET_OS_OSX
        if ((strcmp(what, "openDirectoryDialog") == 0)
            || (strcmp(what, "openFileDialog") == 0)
            || (strcmp(what, "saveFileDialog") == 0))
        {
            if (args.size() <= 2) {
                /* INFO SHOWN IN GIDEROS STUDIO DEBUGGER, IMPLEMENTED IN QT, NOT NEEDED HERE? */
            }
            else
            {
                std::string title = args[0].s;
                std::string place = args[1].s;
                
                std::vector<std::string> filters;
                if (args.size() >= 3) {
                    std::string ext = args[2].s;
                    while (!ext.empty()) {
                        std::string next;
                        size_t semicolon = ext.find(";;");
                        if (semicolon != std::string::npos) {
                            next = ext.substr(semicolon + 2);
                            ext = ext.substr(0, semicolon);
                        }
                        size_t p1 = ext.find_first_of('(');
                        size_t p2 = ext.find_last_of(')');
                        if ((p1 != std::string::npos) && (p2 != std::string::npos) && (p2 > p1))
                        {
                            //Valid filter, extract label and extensions
                            std::string label = ext.substr(0, p1);
                            std::string exts = ext.substr(p1 + 1, p2 - p1 - 1);
                            size_t semicolon;
                            while ((semicolon = exts.find(" "))!=std::string::npos)
                            {
                                std::string e = exts.substr(0, semicolon);
                                size_t dot = e.find(".");
                                if (dot != std::string::npos)
                                    e = e.substr(dot+1);
                                exts= exts.substr(semicolon + 1);
                                filters.push_back(e);
                            }
                            size_t dot = exts.find(".");
                            if (dot != std::string::npos)
                                exts = exts.substr(dot+1);
                            filters.push_back(exts);
                        }
                        ext = next;
                    }
                }
                size_t nfilters=filters.size();
                NSMutableArray<UTType *> *ftypes=nil;
                if (@available (macOS 11, *)) {
                    ftypes=[NSMutableArray<UTType *> arrayWithCapacity:nfilters];
                    for (size_t i=0;i<nfilters;i++) {
                        ftypes[0]=[UTType typeWithFilenameExtension:[NSString stringWithUTF8String:filters[i].c_str()]];
                    }
                }

                if (strcmp(what, "openDirectoryDialog") == 0) {
                    NSOpenPanel* panel = [NSOpenPanel openPanel];
                    panel.canChooseFiles=FALSE;
                    panel.canChooseDirectories=TRUE;
                    panel.allowsMultipleSelection=FALSE;
                    panel.title=[NSString stringWithUTF8String:title.c_str()];

                    if ([panel runModal]==NSFileHandlingPanelOKButton)
                    {
                        r.type = gapplication_Variant::STRING;
                        r.s = [[panel URL] fileSystemRepresentation];
                        rets.push_back(r);
                    }
                }
                else if (strcmp(what, "openFileDialog") == 0) {
                    NSOpenPanel* panel = [NSOpenPanel openPanel];
                    panel.canChooseFiles=TRUE;
                    panel.canChooseDirectories=FALSE;
                    panel.allowsMultipleSelection=FALSE;
                    panel.title=[NSString stringWithUTF8String:title.c_str()];
                    if (@available (macOS 11, *))
                        panel.allowedContentTypes=ftypes;
                    
                    if ([panel runModal]==NSFileHandlingPanelOKButton)
                    {
                        r.type = gapplication_Variant::STRING;
                        r.s = [[panel URL] fileSystemRepresentation];
                        rets.push_back(r);
                    }
                }
                else if (strcmp(what, "saveFileDialog") == 0) {
                    NSSavePanel* panel = [NSSavePanel savePanel];
                    panel.title=[NSString stringWithUTF8String:title.c_str()];
                    if (@available (macOS 11, *))
                        panel.allowedContentTypes=ftypes;
                    
                    if ([panel runModal]==NSFileHandlingPanelOKButton)
                    {
                        r.type = gapplication_Variant::STRING;
                        r.s = [[panel URL] fileSystemRepresentation];
                        rets.push_back(r);
                    }
                }
            }
            /*------------------------------------------------------------------*/
        }
#endif
	}
    else {
#if TARGET_OS_OSX
        if (!strcmp(what, "cursor")) {
            NSCursor *mapped=[NSCursor arrowCursor];
            if (args[0].s=="arrow") mapped=[NSCursor arrowCursor];
            else if (args[0].s=="cross") mapped=[NSCursor crosshairCursor];
            else if (args[0].s=="IBeam") mapped=[NSCursor IBeamCursor];
            else if (args[0].s=="sizeHor") mapped=[NSCursor resizeLeftRightCursor];
            else if (args[0].s=="sizeVer") mapped=[NSCursor resizeUpDownCursor];
            else if (args[0].s=="splitH") mapped=[NSCursor resizeLeftRightCursor];
            else if (args[0].s=="splitV") mapped=[NSCursor resizeUpDownCursor];
            else if (args[0].s=="pointingHand") mapped=[NSCursor pointingHandCursor];
            else if (args[0].s=="forbidden") mapped=[NSCursor operationNotAllowedCursor];
            else if (args[0].s=="openHand") mapped=[NSCursor openHandCursor];
            else if (args[0].s=="closedHand") mapped=[NSCursor closedHandCursor];
            else if (args[0].s=="dragCopy") mapped=[NSCursor dragCopyCursor];
            else if (args[0].s=="dragLink") mapped=[NSCursor dragLinkCursor];
            [mapped set];
        }
#elif TARGET_OS_IOS
        if (!strcmp(what, "statusBar")) {
            bool hidden=(args.size()==0)||(args[0].type==gapplication_Variant::NIL);
            UIStatusBarStyle style=UIStatusBarStyleDefault;
            if (!hidden) {
                if (args[0].s=="light") style=UIStatusBarStyleLightContent;
                if (@available (iOS 13, *)) {
                    if (args[0].s=="dark") style=UIStatusBarStyleDarkContent;
                }
            }
            UIViewController *viewController = g_getRootViewController();

            [viewController setStatusBar:hidden style:style];
        }
#endif
    }
	return rets;
}

