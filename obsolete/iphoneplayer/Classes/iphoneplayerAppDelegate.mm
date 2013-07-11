//
//  iphoneplayerAppDelegate.m
//  iphoneplayer
//
//  Created by Atilim Cetin on 2/28/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "iphoneplayerAppDelegate.h"
#import "EAGLView.h"

@implementation iphoneplayerAppDelegate

@synthesize window;
@synthesize glView;

- (void) applicationDidFinishLaunching:(UIApplication *)application
{
	[window makeKeyAndVisible];
	[glView startAnimation];
}

- (void) applicationWillResignActive:(UIApplication *)application
{
	[glView stopAnimation];
}

- (void) applicationDidBecomeActive:(UIApplication *)application
{
	[glView startAnimation];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	[glView stopAnimation];
	[glView exitGameLoop];
}

- (void) dealloc
{
	[window release];
	[glView release];
	
	[super dealloc];
}

@end
