//
//  iphoneplayer3AppDelegate.m
//  iphoneplayer3
//
//  Created by Atilim Cetin on 3/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "iphoneplayer3AppDelegate.h"
#import "iphoneplayer3ViewController.h"

#include "../giderosapi.h"

@implementation iphoneplayer3AppDelegate

@synthesize window;
@synthesize viewController;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
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
