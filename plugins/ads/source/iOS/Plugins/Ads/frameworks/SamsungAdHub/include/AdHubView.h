//
// AdHubView.h
// SamsungAdHub
//
// Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
//

#import "SADBannerView.h"
#import "AdHubUserProfile.h"
#import "AdHubViewDelegate.h"

@class AdHubParameter;
@interface AdHubView : SADBannerView <AdHubViewDelegate> {
AdHubParameter *param;
id delegate;
UIViewController *rootViewController;
NSTimer *retryTimer;
NSString *profileStr;
}

/**
@brief Constructor
@param adSize Banner size constant
@param origin Banner's top left corner coordinate
@param inventoryID Inventory id
@return Banner view object
*/
- (id) initWithAdSize:(AdHubAdSize)adSize origin:(CGPoint)origin inventoryID:(NSString*)inventoryID;

/**
@brief set user profile
@param p user profile object
@remark for Ad targetting, Informations of user like age, gender, religion, etc. can be set.
*/
- (void) setUserProfile:(AdHubUserProfile *)p;

/**
@brief Set ad refresh rate
@param time Refresh interval, unit is ms (1/1000sec)
@remark It can be set by user. If isn't set, value from server is used. Minimum value is 15 sec
*/
- (void) setRefreshRate:(int)time;

/**
@brief Restart refresh timer
*/
- (void)startRefresh;

/**
@brief Stop refresh timer
*/
-(void)stopRefresh;

/**
@brief Start banner ad
*/
- (void)startAd;

/**
@brief Delegate object that should handle event
*/
@property (nonatomic, assign) id delegate;

/**
@brief View controller of banner view
*/
@property (nonatomic, retain) UIViewController *rootViewController;
@end

