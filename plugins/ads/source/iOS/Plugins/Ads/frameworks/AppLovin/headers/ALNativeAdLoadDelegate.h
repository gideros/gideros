//
//  ALNativeAdGroupLoadDelegate.h
//  sdk
//
//  Created by Matt Szaro on 5/21/15.
//
//

#import <Foundation/Foundation.h>
#import "ALNullabilityAnnotations.h"
#import "ALNativeAd.h"

@class ALNativeAdService;

@protocol ALNativeAdLoadDelegate

/**
 *  This method is invoked when the native ad service loads an ad.
 *
 *  @param service      Native ad service which loaded the ad.
 *  @param ads          NSArray containing ALNativeAds which were loaded.
 */
-(void) nativeAdService: (alnonnull ALNativeAdService*) service didLoadAds: (alnonnull NSArray* /* of ALNativeAd */) ads;

/**
 *  This method is invoked when the native ad service fails to load ad.
 *
 *  @param service      Native ad service which loaded the ad.
 *  @param code         An error code corresponding with a constant defined in <code>ALErrorCodes.h</code>.
 */
-(void) nativeAdService: (alnonnull ALNativeAdService*) service didFailToLoadAdsWithError: (NSInteger) code;

@end
