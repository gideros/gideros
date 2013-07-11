//
//  GiderosiPadPlayerAppDelegate.m
//  GiderosiPadPlayer
//
//  Created by Atilim Cetin on 1/18/12.
//  Copyright 2012 Gideros Mobile. All rights reserved.
//

#import "GiderosiPadPlayerAppDelegate.h"
#import "GiderosiPadPlayerViewController.h"

#include "../giderosapi.h"

@implementation GiderosiPadPlayerAppDelegate

@synthesize window;
@synthesize viewController;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	CGRect bounds = [[UIScreen mainScreen] bounds];
	
    self.window = [[[UIWindow alloc] initWithFrame:bounds] autorelease];
	
    self.viewController = [[[GiderosiPadPlayerViewController alloc] init] autorelease];	
    self.viewController.wantsFullScreenLayout = YES;
	
	[self.viewController view];
	
	gdr_initialize(self.viewController.glView, bounds.size.width, bounds.size.height, true);

	[self.window addSubview:self.viewController.view];
    [self.window makeKeyAndVisible];

    return YES;
}

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
    // Handle any background procedures not related to animation here.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Handle any foreground procedures not related to animation here.
}

- (void)dealloc
{
    [viewController release];
    [window release];
    
    [super dealloc];
}

@end
