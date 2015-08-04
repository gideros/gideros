/*
 * Copyright (c) 2015, Heyzap, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * * Neither the name of 'Heyzap, Inc.' nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#import <Foundation/Foundation.h>
#import "HZShowOptions.h"
#import "HZShowOptions.h"

@class HZShowOptions;

@protocol HZAdsDelegate;

/** HZInterstitialAd is responsible for fetching and showing interstitial ads. */
@interface HZInterstitialAd : NSObject

#pragma mark - Showing Ads

/** Shows an interstitial ad. */
+ (void) show;

/**
 *  Shows an interstitial ad for a given tag, if available.
 *
 *  @param tag An identifier for the location of the ad which you can use to disable the ad from your dashboard.
 */
+ (void) showForTag: (NSString *) tag;

/**
 *  Shows an interstitial ad for a given tag, if available.
 *
 *  @param tag An identifier for the location/context of the ad which you can use to disable the ad from your dashboard.
 *  @param completion A block called when the ad is shown or failed to show. result contains whether 
 *         or not the fetch was successful, and if not, error contains the reason why.
 */
+ (void) showForTag:(NSString *)tag completion:(void (^)(BOOL result, NSError *error))completion;

/** Shows an interstitial ad with the given options.
 *
 * @param options HZShowOptions object containing properties for configuring how the ad is shown.
 */
+ (void) showWithOptions: (HZShowOptions *) options;

#pragma mark - Callbacks

/** Sets the delegate to receive the messages listed in the `HZAdsDelegate` protocol.
 
 @param delegate The object to receive the callbacks. 
 */
+ (void) setDelegate: (id<HZAdsDelegate>) delegate;

/** Fetches a new ad from Heyzap.  */
+ (void) fetch;

/**
 *  Fetches an interstitial ad with an optional completion handler
 *
 *  @param completion A block called when the ad is fetched or failed to fetch. result contains whether or not the fetch was successful, and if not, error contains the reason why.
 */
+ (void) fetchWithCompletion: (void (^)(BOOL result, NSError *error))completion;


/**
 *  Fetches an interstitial ad for the given tag.
 *
 *  @param tag An identifier for the location of the ad which you can use to disable the ad from your dashboard.
 */
+ (void) fetchForTag: (NSString *) tag;


/**
 *  Fetches an interstitial ad for the given tag with an optional completion handler
 *
 *  @param tag        An identifier for the location of the ad which you can use to disable the ad from your dashboard.
 *  @param completion A block called when the ad is fetched or failed to fetch. result contains whether or not the fetch was successful, and if not, error contains the reason why.
 */
+ (void) fetchForTag:(NSString *)tag withCompletion: (void (^)(BOOL result, NSError *error))completion;

/** Whether or not an ad is available to show. */
+ (BOOL) isAvailable;


/**
 *  Whether or not an ad is available to show for the given tag.
 *
 *  @param tag An identifier for the location of the ad which you can use to disable the ad from your dashboard.
 *
 *  @return If the ad was available
 */
+ (BOOL) isAvailableForTag: (NSString *) tag;


#pragma mark - Private methods

+ (void) setCreativeID:(int)creativeID;
+ (void)forceTestCreative:(BOOL)forceTestCreative;
+ (void)setCreativeType:(NSString *)creativeType;

@end
