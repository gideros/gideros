//
//  ALAdService.h
//  sdk
//
//  Created by Basil on 2/27/12.
//  Copyright (c) 2013, AppLovin Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ALNullabilityAnnotations.h"

#import "ALAd.h"
#import "ALAdSize.h"
#import "ALAdLoadDelegate.h"
#import "ALAdDisplayDelegate.h"
#import "ALAdUpdateDelegate.h"
#import "ALAdVideoPlaybackDelegate.h"

/**
 * This class is responsible for providing and displaying ads.
 */
@interface ALAdService : NSObject

/**
 * @name Loading and Preloading Ads
 */

/**
 * Fetch a new ad, of a given size, notifying a supplied delegate on completion.
 *
 * @param adSize    Size of an ad to load. Must not be nil.
 * @param delegate  A callback to notify of the fact that the ad is loaded.
 */
-(void) loadNextAd: (alnonnull ALAdSize *) adSize andNotify: (alnullable id<ALAdLoadDelegate>)delegate;

/**
 * Pre-load an ad of a given size in the background, if one is not already available.
 *
 * @param adSize Size of the ad to cache.
 */
-(void) preloadAdOfSize: (alnonnull ALAdSize*) adSize;

/**
 * Check whether an ad of a given size is pre-loaded and ready to be displayed.
 *
 * @param adSize Size of the ad to check for.
 *
 * @return YES if an ad of this size is pre-loaded and ready to display without further network activity. NO if requesting an ad of this size would require fetching over the network.
 */
-(BOOL) hasPreloadedAdOfSize: (alnonnull ALAdSize*) adSize;

/**
 * @name Observing Ad Rotations
 */

/**
 * Add an observer of updates of advertisements of a given size.
 *
 *  @param adListener  Listener to add
 *  @param adSize      Size of ads that the listener is interested in
 */
-(void)addAdUpdateObserver: (alnonnull id<ALAdUpdateObserver>) adListener ofSize: (alnonnull ALAdSize *) adSize;

/**
 * Remove an observer of updates of advertisements of a given size.
 *
 *  @param adListener  Listener to modify
 *  @param adSize      Size of ads that the listener should no longer receive notifications about
 */
-(void)removeAdUpdateObserver: (alnonnull id<ALAdUpdateObserver>) adListener ofSize: (alnonnull ALAdSize *) adSize;

- (alnullable id)init __attribute__((unavailable("Don't instantiate ALAdService, access one via [sdk adService] instead.")));
@end

/**
 * This is an endpoint name for custom AppLovin URL for forcing
 * container to load the next ad:
 * <pre>
 *        applovin://com.applovin.sdk/adservice/next_ad
 * </pre>
 */
extern NSString * const __alnonnull ALSdkUriNextAd;

/**
 * This is an endpoint name for custom AppLovin URL for forcing
 * ad container to close itself:
 * <pre>
 *        applovin://com.applovin.sdk/adservice/close_ad
 * </pre>
 */
extern NSString * const __alnonnull ALSdkCloseAd;

/**
 * This is an endpoint name for custom landing page that should
 * be displayed.
 * <pre>
 *        applovin://com.applovin.sdk/adservice/landing_page/<PAGE_ID>
 * </pre>
 */
extern NSString * const __alnonnull ALSdkLandingPage __deprecated_msg("No longer used.");
