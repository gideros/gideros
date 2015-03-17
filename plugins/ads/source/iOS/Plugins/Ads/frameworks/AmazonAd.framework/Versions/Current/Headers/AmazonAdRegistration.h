//
//  AmazonAdRegistration.h
//  AmazonMobileAdsSDK
//
//  Copyright (c) 2012-2014 Amazon.com. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface AmazonAdRegistration : NSObject

+ (instancetype)sharedRegistration;

// Set the applicationId provided by the Amazon Appstore.
- (void)setAppKey:(NSString *)appKey;

// Enable/Disable logging. Logging is turned off by default.
- (void)setLogging:(BOOL)isEnabled;

// The current Ad SDK version.
- (NSString *)sdkVersion;

// Register your app to track app downloads from mobile ad campaigns. 
// To add app download conversion tracking to your app, invoke the 
// setApplicationId and the registerApp from your applicationDelegate's
// applicationDidBecomeActive protocol method.
// 
// Note: Calling loadAd automatically registers your app. 
// Only call this method if you are not using the Amazon Ads SDK to 
// display Ads, but to track app downloads from mobile ads campaigns.
- (void)registerApp;

@end
