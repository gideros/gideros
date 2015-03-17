//
//  IMNativeDelegate.h
//  InMobi Monetization SDK
//
//  Copyright (c) 2013 InMobi. All rights reserved.
//
/**
 * Implement these methods to get notified about the status of a native ad. These will be reported on the main thread.
 */
#import <Foundation/Foundation.h>
#import "IMError.h"
@class IMNative;
@protocol IMNativeDelegate <NSObject>

@required
/**
 * Once a native ad successfully finishes loading, the delegate will be notified with this method. 
 * At this point, the content of the native ad is filled and is usable by the publisher.
 */
-(void)nativeAdDidFinishLoading:(IMNative*)native;
/**
 * Upon error, the native ad delegate will be notified by this method.
 */
-(void)nativeAd:(IMNative*)native didFailWithError:(IMError*)error;

@end
