//
//  GiderosiPhonePlayerAppDelegate.m
//  GiderosiPhonePlayer
//
//  Created by Atilim Cetin on 6/8/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "GiderosiPhonePlayerAppDelegate.h"
#import "GiderosiPhonePlayerViewController.h"
#import "EAGLView.h"

#include "../giderosapi.h"


@implementation GiderosiPhonePlayerAppDelegate

@synthesize window;
@synthesize viewController;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	UIView* glView = self.viewController.view;
	UIView* rootView = [[UIView alloc] initWithFrame:[self.window bounds]];
	[rootView addSubview:glView];
	self.viewController.view = rootView;
	[rootView release];
    
	gdr_initialize((EAGLView *)glView, 320, 480, true);

#if 0	// waiting for dropping iOS 3.x compatibility
	self.window.rootViewController = self.viewController;
#else
    [self.window addSubview:self.viewController.view];
#endif

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
