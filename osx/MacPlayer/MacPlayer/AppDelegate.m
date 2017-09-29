//
//  AppDelegate.m
//
//  Copyright 2012 Gideros Mobile. All rights reserved.
//

#import "AppDelegate.h"
#import "ViewController.h"

#import <AVFoundation/AVFoundation.h>

#include "giderosapi.h"

//GIDEROS-TAG-MAC:APP-DELEGATE-DECL//

@implementation AppDelegate

@synthesize window;
@synthesize viewController;

- (void) applicationWillFinishLaunching: (NSNotification *)not;
{
     id menubar = [NSMenu new];
     id appMenuItem = [NSMenuItem new];
     [menubar addItem:appMenuItem];
     [NSApp setMainMenu:menubar];
     id appMenu = [NSMenu new];
     id appName = [[NSProcessInfo processInfo] processName];
     id quitTitle = [@"Quit " stringByAppendingString:appName];
     id quitMenuItem = [[NSMenuItem alloc] initWithTitle:quitTitle
     action:@selector(terminate:) keyEquivalent:@"q"];
     [appMenu addItem:quitMenuItem];
     [appMenuItem setSubmenu:appMenu];
}

- (BOOL)isNotRotatedBySystem{
    return true;
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification;
{
	CGRect bounds;
    bounds.origin.x=0;
    bounds.origin.y=0;
    bounds.size.width=320;
    bounds.size.height=480;

    self.window = [[NSWindow alloc] initWithContentRect:bounds styleMask:NSWindowStyleMaskResizable|NSWindowStyleMaskTitled|NSWindowStyleMaskClosable backing:NSBackingStoreBuffered defer:FALSE];
    [window cascadeTopLeftFromPoint:NSMakePoint(20,20)];

    self.viewController = [[ViewController alloc] init];
	[self.viewController view];

    int width = bounds.size.width;
    int height = bounds.size.height;
     
    NSString *path = [[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"assets"] stringByAppendingPathComponent:@"properties.bin"];
    
    NSFileManager *fileManager = [[NSFileManager alloc] init];
    
    BOOL isPlayer = false;
    BOOL exists = [fileManager fileExistsAtPath:path];
    if (!exists) {
        isPlayer = true;
    }
    
    gdr_initialize(self.viewController.glView, width, height, isPlayer);

    //[self.window setRootViewController:self.viewController];
    id appName = [[NSProcessInfo processInfo] processName];
    [window setTitle:appName];
    [self.window setContentView:self.viewController.view];

    [self.window makeKeyAndOrderFront:nil];
    [self.window setAcceptsMouseMovedEvents:TRUE];
    
    gdr_drawFirstFrame();

    gdr_resume();
    [self.viewController startAnimation];

    //GIDEROS-TAG-MAC:APP-LAUNCHED//
}

#if __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_4_2
- (BOOL)application:(NSApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation
{
    gdr_handleOpenUrl(url);
    return YES;
}
#else
- (BOOL)application:(NSApplication *)application handleOpenURL:(NSURL *)url
{
    gdr_handleOpenUrl(url);
    return YES;
}
#endif

- (void)applicationWillResignActive:(NSApplication *)application
{
    gdr_background();
	//gdr_suspend();
    //[self.viewController stopAnimation];
}

- (void)applicationDidBecomeActive:(NSApplication *)application
{
    gdr_foreground();
	//gdr_resume();
    //[self.viewController startAnimation];
}

- (void)applicationWillTerminate:(NSApplication *)application
{
	gdr_exitGameLoop();
    [self.viewController stopAnimation];
	gdr_deinitialize();
}

- (void)application:(NSApplication *)application didReceiveRemoteNotification:(NSDictionary *)userInfo {
    //GIDEROS-TAG-MAC:NOTIFICATION-RX//
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed: (NSApplication *) theApplication {
    return TRUE;
}

@end
