//
// AdHubBarBannerView.h
// SamsungAdHub
//
// Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
//

#import "SADBannerView.h"
#import "AdHubView.h"
#import "AdHubUserProfile.h"
#import "SADBannerViewDelegate.h"

@class AdHubParameter;
@interface AdHubBarBannerView : UIView <SADBannerViewDelegate> {
CGRect frameLandscape;
CGRect framePortrait;
AdHubView *bannerView;
AdHubParameter *param;
id delegate;
UIViewController *rootViewController;
NSTimer *retryTimer;
NSString *profileStr;
CGFloat width;
}

/**
@brief Constructor
@param adSize Constant that specifies ad size
@param origin Banner view's lefu-up corner point
@param inventoryID Inventory ID
@return Banner view object
*/
- (id) initWithAdSize:(AdHubAdSize)adSize yPositionPortrait:(CGFloat)yp yPositionLandscape:(CGFloat)yl inventoryID:(NSString*)inventoryID;

/**
@brief Set user profile
@param p User profile object
@remark 광고소비자의 나이,성별,종교,최미,관심사 설정
*/
- (void) setUserProfile:(AdHubUserProfile *)p;

/**
@brief Set refresh timer interval
@param time Interval, unit is ms (1/1000 sec)
@remark SDK user can set interval. if not, value is fetched from server. minimum value is 15 sec.
*/
- (void) setRefreshRate:(int)time;

/**
@brief Start refresh timer
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
@brief Set portrait mode
*/
-(void)setPortrait;

/**
@brief Set portrait mode
*/
-(void)setLandscape;

/**
@brief Delegate object that handles events
*/
@property (nonatomic, assign) id delegate;

/**
@brief Root view controller of banner view.
*/
@property (nonatomic, retain) UIViewController *rootViewController;
@end

