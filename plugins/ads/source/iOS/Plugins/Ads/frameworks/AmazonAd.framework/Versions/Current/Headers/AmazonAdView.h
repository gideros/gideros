//
//  AmazonAdView.h
//  AmazonMobileAdsSDK
//
//  Copyright (c) 2012-2014 Amazon.com. All rights reserved.
//

#import <Foundation/Foundation.h>


@class AmazonAdError;
@class AmazonAdOptions;
@protocol AmazonAdViewDelegate;

@interface AmazonAdView : UIView

@property (nonatomic, unsafe_unretained) id<AmazonAdViewDelegate> delegate;

// Create an Ad view and instantiate it using one of the standard AdSize options specified in AmazonAdOptions
+ (instancetype)amazonAdViewWithAdSize:(CGSize)adSize;

// Instantiate using one of the standard AdSize options specified in AmazonAdOptions. 
- (instancetype)initWithAdSize:(CGSize)adSize;

// Loads an Ad in this view
- (void)loadAd:(AmazonAdOptions *)options;

// Returns YES if the Ad in this view is expanded
- (BOOL)isAdExpanded;

@end

@protocol AmazonAdViewDelegate <NSObject>

@required

/*
 * The ad view relies on this method to determine which view controller will be 
 * used for presenting/dismissing modal views, such as the browser view presented 
 * when a user clicks on an ad.
 */
- (UIViewController *)viewControllerForPresentingModalView;

@optional

/*
 * These callbacks are triggered when the ad view is about to present/dismiss a
 * modal view. If your application may be disrupted by these actions, you can
 * use these notifications to handle them.
 */
- (void)adViewWillExpand:(AmazonAdView *)view;
- (void)adViewDidCollapse:(AmazonAdView *)view;

/*
 * These callbacks notify you whether the ad view (un)successfully loaded an ad.
 */
- (void)adViewDidFailToLoad:(AmazonAdView *)view withError:(AmazonAdError *)error;
- (void)adViewDidLoad:(AmazonAdView *)view;

@end
