#if TARGET_OS_OSX
#import <Cocoa/Cocoa.h>
#define UIApplication NSApplication
#define UIViewController NSViewController
#else
#import <UIKit/UIKit.h>
#endif

extern "C" {

UIViewController* g_getRootViewController()
{
	//	return [UIApplication sharedApplication].keyWindow.rootViewController;
	return [[UIApplication sharedApplication].delegate viewController];
}

}
