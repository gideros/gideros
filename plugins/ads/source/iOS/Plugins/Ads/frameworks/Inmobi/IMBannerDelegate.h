//
//  IMBannerDelegate.h
//  InMobi Monetization SDK
//
//  Copyright (c) 2013 InMobi. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "IMError.h"

@class IMBanner;

/**
 * This is the delegate for receiving state change messages from an IMBanner.
 * Use this to receive callbacks for banner ad request succeeding, failing or
 * for the events after the banner ad is clicked.
 */
@protocol IMBannerDelegate <NSObject>

@optional

#pragma mark Banner Request Notifications
/**
 * Callback sent when an ad request loaded an ad. This is a good opportunity
 * to add this view to the hierarchy if it has not yet been added.
 * @param banner The IMBanner instance which finished loading the ad request.
 */
- (void)bannerDidReceiveAd:(IMBanner *)banner;
/**
 * Callback sent when an ad request failed. Normally this is because no network
 * connection was available or no ads were available (i.e. no fill).
 * @param banner The IMBanner instance that failed to load the ad request.
 * @param error The error that occurred during loading.
 */
- (void)banner:(IMBanner *)banner didFailToReceiveAdWithError:(IMError *)error;

#pragma mark Banner Interaction Notifications
/**
 * Called when the banner is tapped or interacted with by the user
 * Optional data is available to publishers to act on when using
 * monetization platform to render promotional ads.
 * @param banner The IMBanner instance that presents the screen.
 * @param dictionary The NSDictionary containing the parameters as passed by the creative
 */
-(void)bannerDidInteract:(IMBanner *)banner withParams:(NSDictionary *)dictionary;
/**
 * Callback sent just before when the banner is presenting a full screen view
 * to the user. Use this opportunity to stop animations and save the state of
 * your application in case the user leaves while the full screen view is on
 * screen (e.g. to visit the App Store from a link on the full screen view).
 * @param banner The IMBanner instance that presents the screen.
 */
- (void)bannerWillPresentScreen:(IMBanner *)banner;
/**
 * Callback sent just before dismissing the full screen view.
 * @param banner The IMBanner instance that dismisses the screen.
 */
- (void)bannerWillDismissScreen:(IMBanner *)banner;
/**
 * Callback sent just after dismissing the full screen view.
 * Use this opportunity to restart anything you may have stopped as part of
 * bannerWillPresentScreen: callback.
 * @param banner The IMBanner instance that dismissed the screen.
 */
- (void)bannerDidDismissScreen:(IMBanner *)banner;
/**
 * Callback sent just before the application goes into the background because
 * the user clicked on a link in the ad that will launch another application
 * (such as the App Store). The normal UIApplicationDelegate methods like
 * applicationDidEnterBackground: will immediately be called after this.
 * @param banner The IMBanner instance that is launching another application.
 */
- (void)bannerWillLeaveApplication:(IMBanner *)banner;

@end
