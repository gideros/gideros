//
//  AppDelegate.m
//
//  Copyright © 2020 Gideros Mobile. All rights reserved.
//

#import "AppDelegate.h"
#import "ViewController.h"

#import <AVFoundation/AVFoundation.h>

#define UIView NSView
#include "giderosapi.h"

@implementation AppDelegate

@synthesize window;
@synthesize viewController;

- (BOOL)isNotRotatedBySystem{
    return true;
}


- (void) applicationDidFinishLaunching:(NSNotification *)launchOptions
{
    NSArray *path1 = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    NSString *cachesDirectory = [path1 objectAtIndex:0];
        
    [self copyUserDefaultsToCache];
    
    CGRect bounds = CGRectMake(100,100, 240,320);
 
    self.viewController = [[ViewController alloc] init];
    self.window = [NSWindow windowWithContentViewController:self.viewController];
    self.window.delegate=self.viewController;

    int height = bounds.size.width;
    int width = bounds.size.height;
    
    
    NSString *path = [[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"assets"] stringByAppendingPathComponent:@"properties.bin"];
    
    NSFileManager *fileManager = [[NSFileManager alloc] init];
    
    BOOL isPlayer = false;
    BOOL exists = [fileManager fileExistsAtPath:path];
    if (!exists) {
        isPlayer = true;
    }
    
    gdr_initialize([self.viewController getGlView], width, height, isPlayer);
    /*
    if ([[[UIDevice currentDevice] systemVersion] compare:@"6.0" options:NSNumericSearch] != NSOrderedAscending)
    {
        [self.window setRootViewController:self.viewController];
    }
    else
    {
        [self.window addSubview:self.viewController.view];
    }
    
    [self.window makeKeyAndVisible];
    */
    
    [window makeKeyAndOrderFront:NSApp];
    gdr_drawFirstFrame();
    
    gdr_resume();
    [self.viewController startAnimation];
}

- (BOOL)application:(NSApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation
{
    gdr_handleOpenUrl(url);
    return YES;
}

- (void)applicationWillResignActive:(NSApplication *)application
{
    gdr_background();
}

- (void)applicationDidBecomeActive:(NSApplication *)application
{
    //gdr_resume();
    gdr_foreground();
}

- (void)applicationWillTerminate:(NSApplication *)application
{
    gdr_exitGameLoop();
    [self.viewController stopAnimation];
    gdr_deinitialize();
    
    [self copyCacheToUserDefaults];
}

- (void)applicationDidEnterBackground:(NSApplication *)application
{
    [self copyCacheToUserDefaults];
}

- (void)applicationWillEnterForeground:(NSApplication *)application
{
}

- (void) clearUserDefaults {
    NSDictionary * myDefaults = [[NSUserDefaults standardUserDefaults] dictionaryRepresentation];
    NSArray *keyArr = [myDefaults allKeys];
    for (NSString *key in keyArr)
    {
        NSArray *fileArray = [key componentsSeparatedByString:@"|"];
        if (fileArray && [[fileArray objectAtIndex:0] isEqualToString:@"FILE"]) {
            [[NSUserDefaults standardUserDefaults] removeObjectForKey:key];
        }
    }
}

- (void) copyCacheToUserDefaults {
    [self clearUserDefaults];
    NSArray *path = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    NSString *cachesDirectory = [path objectAtIndex:0];
    NSArray* dirs = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:cachesDirectory
                                                                        error:NULL];
    
    [dirs enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        NSString *filename = [NSString stringWithFormat:@"FILE|%@", (NSString *)obj];
        NSLog(@"Writing file %@ to NSUserDefaults", (NSString *)obj);
        [[NSUserDefaults standardUserDefaults] setObject:[NSData dataWithContentsOfFile:[NSString stringWithFormat:@"%@/%@", cachesDirectory, (NSString *)obj]] forKey:filename];
    }];
    
    NSLog(@"Syncing NSUserDefaults");
    [[NSUserDefaults standardUserDefaults] synchronize];
    
}

- (void) copyUserDefaultsToCache {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    NSString *cachesDirectory = [paths objectAtIndex:0];
    NSDictionary * myDefaults = [[NSUserDefaults standardUserDefaults] dictionaryRepresentation];
    NSArray *keyArr = [myDefaults allKeys];
    for (NSString *key in keyArr)
    {
        NSArray *fileArray = [key componentsSeparatedByString:@"|"];
        NSLog(@"Read key %@", key);

        if (fileArray && [[fileArray objectAtIndex:0] isEqualToString:@"FILE"]) {
            NSString * fileName = [fileArray objectAtIndex:1];
            NSLog(@"Getting file %@ from NSUserDefaults", fileName);
            NSData * thisData = [myDefaults objectForKey:key];
            [thisData writeToFile:[NSString stringWithFormat:@"%@/%@", cachesDirectory, fileName] atomically:NO];
            
        }
    }
}

- (void)dealloc
{
 
}

@end
