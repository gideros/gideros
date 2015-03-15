//
//  AppLovinSdk.h
//
//  Created by Basil Shikin on 2/1/12.
//  Copyright (c) 2013, AppLovin Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#import "ALSdkSettings.h"
#import "ALAdService.h"
#import "ALTargetingData.h"

#import "ALErrorCodes.h"

/**
 * This is a base class for the AppLovin iOS SDK.
 *
 */
@interface ALSdk : NSObject

/**
 * @name SDK Configuration
 */

/**
 * This SDK's key.
 */
@property (strong, readonly) NSString* sdkKey;

/**
 * This SDK's settings.
 */
@property (strong, readonly) ALSdkSettings* settings;

/**
 * Set Plugin version.
 *
 * This is mainly used internally, however if you've written a mediation adaptor or plugin,
 * you can set this. Common examples include things like "Bob's Cocos2D Plugin v1.0".
 *
 * @param version Some descriptive string which identifies the plugin.
 */
-(void) setPluginVersion: (NSString *) version;

/**
 * @name SDK Information
 */

/**
 *  Get the current version of the SDK.
 *
 *  @return The current SDK version.
 */
+(NSString *) version;

/**
 * @name SDK Services
 */

/**
 * Get an instance of AppLovin Ad service. This service is
 * used to fetch and display ads from AppLovin servers.
 *
 * @return Ad service. Guaranteed not to be null.
 */
-(ALAdService *) adService;

/**
 * @name Custom User Targeting
 */

/**
 * Get an instance of AppLovin Targeting data. This object contains
 * targeting values that could be provided to AppLovin for better
 * advertisement performance.
 *
 * @return Current targeting data. Guaranteed not to be null.
 */
-(ALTargetingData *) targetingData;

/**
 * @name SDK Initialization
 */

/**
 * Initialize current version of the SDK.
 *
 */
-(void) initializeSdk;

/**
 * Initialize the default instance of AppLovin SDK.
 *
 * Please make sure that application's
 * <code>Info.plist</code> includes a property 'AppLovinSdkKey' that is set to provided SDK key.
 *
 * @return An instance of AppLovinSDK
 */
+(void) initializeSdk;

/**
 * @name Getting SDK Instances
 */

/**
 * Get a shared instance of AppLovin SDK.
 *
 * Please make sure that application's
 * <code>Info.plist</code> includes a property 'AppLovinSdkKey' that is set to provided SDK key.
 *
 * @return An instance of AppLovinSDK
 */
+(ALSdk *) shared;

/**
 * Get an instance of AppLovin SDK using default SDK settings.
 *
 * @param sdkKey         SDK key to use. Must not be nil.
 *
 * @return An instance of AppLovinSDK
 */
+(ALSdk *) sharedWithKey: (NSString *) sdkKey;

/**
 * Get an instance of AppLovin SDK.
 * 
 * @param sdkKey         SDK key to use. Must not be nil.
 * @param settings       User-provided settings. Must not be nil, but can be an empty <code>[[ALSdkSettings alloc] init]</code> object.
 * 
 * @return An instance of AppLovinSDK
 */
+(ALSdk *) sharedWithKey: (NSString *)sdkKey settings: (ALSdkSettings *)settings;

- (id)init __attribute__((unavailable("Use [ALSdk shared] instead of alloc-init pattern.")));

@end
