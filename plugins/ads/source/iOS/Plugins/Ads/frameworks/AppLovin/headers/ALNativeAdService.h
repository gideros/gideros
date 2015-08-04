//
//  ALNativeAdService.h
//  sdk
//
//  Created by Matt Szaro on 5/21/15.
//
//

#import <Foundation/Foundation.h>
#import "ALNullabilityAnnotations.h"
#import "ALNativeAdLoadDelegate.h"
#import "ALNativeAdPrecacheDelegate.h"

@class ALSdk;
@class ALNativeAd;

@interface ALNativeAdService : NSObject

/**
 *  Load a batch of native ads, which are guaranteed not to repeat, asynchronously.
 *
 *  @param  delegate    The native ad load delegate to notify upon completion.
 */
-(void) loadNativeAdGroupOfCount: (NSUInteger) numberOfAdsToLoad andNotify: (alnullable id <ALNativeAdLoadDelegate>) delegate;

/**
 *  Pre-cache image and video resources of a native ad.
 *
 *  @param  ad          The native ad whose resources should be cached.
 *  @param  delegate    The delegate to be notified upon completion.
 */

-(void) precacheResourcesForNativeAd: (alnonnull ALNativeAd*) ad andNotify: (alnullable id <ALNativeAdPrecacheDelegate>) delegate;

@end
