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
#import "ALNativeAdService.h"
#import "ALTargetingData.h"
#import "ALPostbackService.h"

#import "ALNullabilityAnnotations.h"
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
@property (strong, nonatomic, readonly) NSString* __alnonnull sdkKey;

/**
 * This SDK's settings.
 */
@property (strong, nonatomic, readonly) ALSdkSettings* __alnonnull settings;

/**
 * Set Plugin version.
 *
 * This is mainly used internally, however if you've written a mediation adaptor or plugin,
 * you can set this. Common examples include things like "Bob's Cocos2D Plugin v1.0".
 *
 * @param version Some descriptive string which identifies the plugin.
 */
-(void) setPluginVersion: (alnonnull NSString *) version;

/**
 * @name SDK Information
 */

/**
 *  Get the current version of the SDK.
 *
 *  @return The current SDK version.
 */
+(alnonnull NSString *) version;

/**
 * @name SDK Services
 */

/**
 * Get an instance of AppLovin Ad service. This service is
 * used to fetch and display ads from AppLovin servers.
 *
 * @return Ad service. Guaranteed not to be null.
 */
@property (strong, nonatomic, readonly) ALAdService* __alnonnull  adService;

/**
 * Get an instance of AppLovin Native Ad service. This service is
 * used to fetch and display native ads from AppLovin servers.
 *
 * @return Native ad service. Guaranteed not to be null.
 */
@property (strong, nonatomic, readonly) ALNativeAdService* __alnonnull nativeAdService;

/**
 * Get an instance of the AppLovin postback service. This service is used to dispatch HTTP GET postbacks to arbitrary URLs.
 *
 * @return Postback service. Guaranteed not to be null.
 */
@property (strong, nonatomic, readonly) ALPostbackService*  __alnonnull postbackService;

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
@property (strong, nonatomic, readonly) ALTargetingData* __alnonnull targetingData;

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
+(alnullable ALSdk *) shared;

/**
 * Get an instance of AppLovin SDK using default SDK settings.
 *
 * @param sdkKey         SDK key to use. Must not be nil.
 *
 * @return An instance of AppLovinSDK
 */
+(alnullable ALSdk *) sharedWithKey: (alnonnull NSString *) sdkKey;

/**
 * Get an instance of AppLovin SDK.
 * 
 * @param sdkKey         SDK key to use. Must not be nil.
 * @param settings       User-provided settings. Must not be nil, but can be an empty <code>[[ALSdkSettings alloc] init]</code> object.
 * 
 * @return An instance of AppLovinSDK
 */
+(alnullable ALSdk *) sharedWithKey: (alnonnull NSString *) sdkKey settings: (alnonnull ALSdkSettings *) settings;

- (alnullable id) init __attribute__((unavailable("Use [ALSdk shared] instead of alloc-init pattern.")));

@end
