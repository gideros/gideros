//
//  ALSdkSettings.h
//
//  Copyright (c) 2013, AppLovin Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>

/**
 * This class contains settings for the AppLovin SDK.
 */
@interface ALSdkSettings : NSObject 

/**
 * Toggle verbose logging of AppLovin SDK. 
 * 
 * If enabled AppLovin messages will appear in standard application log accessible via logcat. 
 * All log messages will have "AppLovinSdk" tag.
 * 
 * Verbose logging is <i>disabled</i> by default.
 * 
 * @param isVerboseLoggingEnabled YES if log messages should be output. NO if the SDK should be silent (recommended for App Store submissions).
 */
@property (assign, atomic) BOOL isVerboseLogging;

/**
 * Defines sizes of ads that should be automatically preloaded.
 * <p>
 * Auto preloading is enabled for <code>BANNER,INTER</code> by default.
 * To disable outright, set to "NONE".
 *
 * @param autoPreloadAdSizes Comma-separated list of sizes to preload. For example: "BANNER,INTER"
 */
@property (strong, atomic) NSString * autoPreloadAdSizes;

/**
 * Defines types of ads that should be automatically preloaded.
 * <p>
 * Auto preloading is enabled for <code>REGULAR,INCENTIVIZED</code> by default.
 * To disable outright, set to "NONE".
 *
 * @param autoPreloadAdSizes Comma-separated list of sizes to preload. For example: "REGULAR,INCENTIVIZED"
 */
@property (strong, atomic) NSString * autoPreloadAdTypes;

@end
