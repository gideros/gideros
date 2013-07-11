//
//  FlurryAppCircle.h
//  Flurry iPhone AppCircle Agent
//
//  Copyright 2011 Flurry, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>


@class FlurryOffer;


@interface FlurryAppCircle : NSObject
{}

/*
 enable App Ads. default is NO.
 */
+ (void)setAppCircleEnabled:(BOOL)value;

/*
 returns whether there is an app ad available to show
 */
+ (BOOL)appAdIsAvailable:(NSString *)hook;

/*
 returns the number of App Ads available for the hook
 */
+ (int)getAppAdCount:(NSString*)hook;

/* 
 create an AppCircle banner on a hook and a view parent 
 subsequent calls will return the same banner for the same hook and parent until removed with the API
 */
+ (UIView *)getHook:(NSString *)hook xLoc:(int)x yLoc:(int)y view:(UIView *)view;

/* 
 create an AppCircle banner on a hook and view parent using optional parameters 
 */
+ (UIView *)getHook:(NSString *)hook xLoc:(int)x yLoc:(int)y view:(UIView *)view attachToView:(BOOL)attachToView orientation:(NSString *)orientation canvasOrientation:(NSString *)canvasOrientation autoRefresh:(BOOL)refresh canvasAnimated:(BOOL)canvasAnimated rewardMessage:(NSString *)rewardMessage userCookies:(NSDictionary*)userCookies;

/* 
 update an existing AppCircle banner with a new ad
 */
+ (void)updateHook:(UIView *)banner;

/* 
 remove an existing AppCircle banner from its hook and parent
 a new banner can be created on the same hook and parent after the existing banner is removed
 */
+ (void)removeHook:(UIView *)banner;

/*
 open the canvas without using a banner
 */
+ (void)openCatalog:(NSString *)hook canvasOrientation:(NSString *)canvasOrientation canvasAnimated:(BOOL)canvasAnimated;

/*
 open the fullscreen App Ad takeover window
 */
+ (void)openTakeover:(NSString *)hook orientation:(NSString *)orientation rewardImage:(UIImage *)image rewardMessage:(NSString *)message userCookies:(NSDictionary*)userCookies;

/* 
 obtain basic Ad Offer information to create ads with your own custom theme or look and feel 
 */
+ (BOOL)getOffer:(NSString*)hookName withFlurryOfferContainer:(FlurryOffer*)flurryOffer;

/* 
 obtain basic Ad Offer information to create ads with your own custom theme or look and feel and include additional params to track with the referral URL 
 */
+ (BOOL)getOffer:(NSString*)hookName withFlurryOfferContainer:(FlurryOffer*)flurryOffer userCookies:(NSDictionary*)userCookies;

/*
 obtain a look at meta data for next ad to customize cookies or offers based on that value 
 */
+ (BOOL)peekOffer:(NSString*)hookName withFlurryOfferContainer:(FlurryOffer*)flurryOffer;

/* 
 return the number of Ad Offers available for the hookName 
 */
+ (int)getOfferCount:(NSString*)hookName;

/*
 refer to FlurryAdDelegate.h for delegate details
 */
+ (void)setAppCircleDelegate:(id)delegate;

@end
