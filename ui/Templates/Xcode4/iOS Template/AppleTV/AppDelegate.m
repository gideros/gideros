//
//  AppDelegate.m
//
//  Copyright 2012 Gideros Mobile. All rights reserved.
//

#import "AppDelegate.h"
#import "ViewController.h"

#import <AVFoundation/AVFoundation.h>

#include "giderosapi.h"

#ifndef NSFoundationVersionNumber_iOS_7_1
# define NSFoundationVersionNumber_iOS_7_1 1047.25
#endif

@implementation AppDelegate

@synthesize window;
@synthesize viewController;

- (BOOL)isNotRotatedBySystem{
    BOOL OSIsBelowIOS8 = [[[UIDevice currentDevice] systemVersion] floatValue] < 8.0;
    BOOL SDKIsBelowIOS8 = floor(NSFoundationVersionNumber) <= NSFoundationVersionNumber_iOS_7_1;
    return OSIsBelowIOS8 || SDKIsBelowIOS8;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    NSArray *path1 = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    NSString *cachesDirectory = [path1 objectAtIndex:0];
        
    [self copyUserDefaultsToCache];
    
    CGRect bounds = [[UIScreen mainScreen] bounds];
    
    self.window = [[UIWindow alloc] initWithFrame:bounds];
    
    self.viewController = [[ViewController alloc] init];
    
    [self.viewController view];
    
    [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryAmbient error:nil];
    
    int height = bounds.size.width;
    int width = bounds.size.height;
    
    
    NSString *path = [[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"assets"] stringByAppendingPathComponent:@"properties.bin"];
    
    NSFileManager *fileManager = [[NSFileManager alloc] init];
    
    BOOL isPlayer = false;
    BOOL exists = [fileManager fileExistsAtPath:path];
    if (!exists) {
        isPlayer = true;
        
        NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString* dir = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"gideros"];
        
        NSArray *files = [fileManager contentsOfDirectoryAtPath:dir error:nil];
        if (files != nil) {
            for (NSString *file in files) {
                [self.viewController addProject:file];
            }
        }
        [self.viewController initTable];
    }
    
    gdr_initialize(self.viewController.glView, width, height, isPlayer);
    
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
    
    [self copyCacheToUserDefaults];
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    gdr_background();
    [self copyCacheToUserDefaults];
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    gdr_foreground();
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
