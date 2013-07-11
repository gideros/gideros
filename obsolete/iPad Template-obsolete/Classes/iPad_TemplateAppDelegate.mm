//
//  iPad_TemplateAppDelegate.m
//  iPad Template
//
//  Created by Atilim Cetin on 6/9/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "iPad_TemplateAppDelegate.h"
#import "iPad_TemplateViewController.h"

#include "../giderosapi.h"

@implementation iPad_TemplateAppDelegate

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
