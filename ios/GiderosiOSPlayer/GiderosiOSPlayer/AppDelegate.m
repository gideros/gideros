//
//  AppDelegate.m
//
//  Copyright 2012 Gideros Mobile. All rights reserved.
//

#import "AppDelegate.h"
#import "ViewController.h"

#import <AVFoundation/AVFoundation.h>

#include "giderosapi.h"

@implementation AppDelegate

@synthesize window;
@synthesize viewController;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	CGRect bounds = [[UIScreen mainScreen] bounds];
	
    self.window = [[[UIWindow alloc] initWithFrame:bounds] autorelease];
	
    self.viewController = [[[ViewController alloc] init] autorelease];	
    self.viewController.wantsFullScreenLayout = YES;

	[self.viewController view];

    [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategorySoloAmbient error:nil];

	gdr_initialize(self.viewController.glView, bounds.size.width, bounds.size.height, true);

    if ([[[UIDevice currentDevice] systemVersion] compare:@"6.0" options:NSNumericSearch] != NSOrderedAscending)
    {
        [self.window setRootViewController:self.viewController];
    }
    else
    {
        [self.window addSubview:self.viewController.view];
    }

    [self.window makeKeyAndVisible];

    gdr_drawFirstFrame();

    return YES;
}

#if __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_4_2
- (BOOL)application:(UIApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation
{
    gdr_handleOpenUrl(url);
    return YES;
}
#else
- (BOOL)application:(UIApplication *)application handleOpenURL:(NSURL *)url
{
    gdr_handleOpenUrl(url);
    return YES;
}
#endif

- (void)applicationWillResignActive:(UIApplication *)application
{
	gdr_suspend();
    [self.viewController stopAnimation];
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
	gdr_resume();
    [self.viewController startAnimation];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	gdr_exitGameLoop();
    [self.viewController stopAnimation];
	gdr_deinitialize();
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    gdr_background();
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    gdr_foreground();
}

- (void)dealloc
{
    [viewController release];
    [window release];
    
    [super dealloc];
}

@end
