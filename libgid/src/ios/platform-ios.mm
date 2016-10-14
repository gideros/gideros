#import <UIKit/UIKit.h>
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
	result.push_back([getSysInfoByName("hw.machine") UTF8String]);
	
	return result;
}

void setWindowSize(int width, int height){

}

void setFullScreen(bool fullScreen){

}

std::string getDeviceName(){
    return [[[UIDevice currentDevice] name] UTF8String];
}

void vibrate(int ms)
{
	AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
}

void openUrl(const char* url)
{
	[[UIApplication sharedApplication] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
}

bool canOpenUrl(const char *url)
{
	return [[UIApplication sharedApplication] canOpenURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
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
	[[UIApplication sharedApplication] setIdleTimerDisabled:(awake ? YES : NO)];
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

bool g_checkStringProperty(bool isSet, const char* what){
    return false;
}

void g_setProperty(const char* what, const char* arg){

}

const char* g_getProperty(const char* what, const char* arg){
	return "";
}
