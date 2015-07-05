//
//  ALNativeAdPrecacheDelegate.h
//  sdk
//
//  Created by Matt Szaro on 5/21/15.
//
//

#import <Foundation/Foundation.h>
#import "ALNullabilityAnnotations.h"
#import "ALNativeAd.h"

@class ALNativeAdService;
@protocol ALNativeAdPrecacheDelegate <NSObject>

/**
 *  This method is invoked when the image resources of a native ad finish precaching.
 *
 *  @param service      Native ad service which loaded the ad.
 *  @param ad           Ad in which resources finished precaching.
 */

-(void) nativeAdService: (alnonnull ALNativeAdService*) service didPrecacheImagesForAd: (alnonnull ALNativeAd*) ad;

/**
 *  This method is invoked when the video of a native ad finishes precaching.
 *
 *  Note that if the ad does not contain a video, this callback will not be invoked.
 *
 *  @param service      Native ad service which loaded the ad.
 *  @param ad           Ad in which video resource finished precaching.
 */

-(void) nativeAdService: (alnonnull ALNativeAdService*) service didPrecacheVideoForAd: (alnonnull ALNativeAd*) ad;

/**
 *  This method is invoked when precaching image resources fails.
 *
 *  @param service      Native ad service which loaded the ad.
 *  @param ad           Ad in which resources finished precaching.
 *  @param errorCode    An error code corresponding with a constant defined in <code>ALErrorCodes.h</code>.
 */

-(void) nativeAdService: (alnonnull ALNativeAdService*) service didFailToPrecacheImagesForAd: (alnonnull ALNativeAd*) ad withError: (NSInteger) errorCode;

/**
 *  This method is invoked when precaching a video fails.
 *
 *  Note that if the ad does not contain a video, this callback will not be invoked.
 *
 *  @param service      Native ad service which loaded the ad.
 *  @param ad           Ad in which resources finished precaching.
 *  @param errorCode    An error code corresponding with a constant defined in <code>ALErrorCodes.h</code>.
 */

-(void) nativeAdService: (alnonnull ALNativeAdService*) service didFailToPrecacheVideoForAd: (alnonnull ALNativeAd*) ad withError: (NSInteger) errorCode;

@end
