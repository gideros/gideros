#if TARGET_OS_OSX
#import <Cocoa/Cocoa.h>
#define UIApplication NSApplication
#define UIViewController NSViewController
#else
#import <UIKit/UIKit.h>
#endif
#import <AudioToolbox/AudioToolbox.h>

#include <vector>
#include <string>

#include <sys/socket.h> // Per msqr
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_dl.h>

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
#endif
	}
	return rets;
}

