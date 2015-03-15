//
// AdHubViewDelegate.h
// SamsungAdHub
//
// Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>

@class AdHubView, AdHubRequestError;
@protocol AdHubViewDelegate <NSObject>
@optional
- (void)bannerViewWillLoadAd:(AdHubView *)banner;

/**
@brief It is called after banner is loaded
*/
- (void)bannerViewDidLoadAd:(AdHubView *)banner;

/**
@brief It is called when error occurs in banner ad
*/
- (void)bannerView:(AdHubView *)banner didFailToReceiveAdWithError:(AdHubRequestError *)error;
@end

