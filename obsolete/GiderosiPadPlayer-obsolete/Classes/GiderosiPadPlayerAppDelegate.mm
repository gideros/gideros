//
//  GiderosiPadPlayerAppDelegate.m
//  GiderosiPadPlayer
//
//  Created by Atilim Cetin on 6/8/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "GiderosiPadPlayerAppDelegate.h"
#import "GiderosiPadPlayerViewController.h"

#include "../giderosapi.h"

@implementation GiderosiPadPlayerAppDelegate

@synthesize window;
@synthesize viewController;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    //self.window.rootViewController = self.viewController;
	[self.window addSubview:self.viewController.view];
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
