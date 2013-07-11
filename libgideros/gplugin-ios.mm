#import <UIKit/UIKit.h>

extern "C" {

UIViewController* g_getRootViewController()
{
	//	return [UIApplication sharedApplication].keyWindow.rootViewController;
	return [[UIApplication sharedApplication].delegate viewController];
}

}
