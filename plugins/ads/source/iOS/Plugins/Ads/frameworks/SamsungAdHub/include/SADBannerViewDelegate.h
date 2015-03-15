//
// SADBannerViewDelegate.h
// SamsungAdHub
//
// Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>

@class SADBannerView, AdHubRequestError;
@protocol SADBannerViewDelegate <NSObject>
@optional
- (void)bannerViewWillLoadAd:(SADBannerView *)banner;
- (void)bannerViewDidLoadAd:(SADBannerView *)banner;
- (void)bannerView:(SADBannerView *)banner didFailToReceiveAdWithError:(AdHubRequestError *)error;
- (BOOL)bannerViewActionShouldBegin:(SADBannerView *)banner willLeaveApplication:(BOOL)willLeave;
- (void)bannerViewActionDidFinish:(SADBannerView *)banner;
- (void)bannerViewActionDidClose:(SADBannerView *)banner;
@end

