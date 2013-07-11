//
//  iphoneplayer2AppDelegate.m
//  iphoneplayer2
//
//  Created by Atilim Cetin on 12/7/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "iphoneplayer2AppDelegate.h"
#import "iphoneplayer2ViewController.h"

@implementation iphoneplayer2AppDelegate

@synthesize window;
@synthesize viewController;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	printf("didFinishLaunchingWithOptions\n");
    [self.window addSubview:self.viewController.view];
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    [self.viewController stopAnimation];
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    [self.viewController startAnimation];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	printf("applicationWillTerminate\n");
    [self.viewController stopAnimation];
	[self.viewController exitGameLoop];
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
	printf("applicationDidEnterBackground\n");
    // Handle any background procedures not related to animation here.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
	printf("applicationWillEnterForeground\n");
    // Handle any foreground procedures not related to animation here.
}

- (void)dealloc
{
    [viewController release];
    [window release];
    
    [super dealloc];
}

@end
