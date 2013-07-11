#import <UIKit/UIKit.h>

void openUrl(const char* url)
{
	[[UIApplication sharedApplication] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
}

bool canOpenUrl(const char *url)
{
	return [[UIApplication sharedApplication] canOpenURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
}