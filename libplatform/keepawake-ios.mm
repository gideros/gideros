#import <UIKit/UIKit.h>

void setKeepAwake(bool awake)
{
	[[UIApplication sharedApplication] setIdleTimerDisabled:(awake ? YES : NO)];
}
