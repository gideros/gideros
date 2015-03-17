//
//  IMNative.h
//  InMobi Monetization SDK
//
//  Copyright (c) 2013 InMobi. All rights reserved.
//
//
/**
 This class represents a Native ad. A minimum implementation to
 get an ad is:
 
   - Initialize an IMNative instance by providing appid
   - Load the Native ad.
   - Implement the IMNativeDelegate methods to know when to get the ad content.
 
 Below is a sample example:
 
        IMNative* nativeAd = [[IMBanner alloc] initWithAppId:@"YOUR_APP_ID"];
        nativeAd.delegate = self;
        [nativeAd loadAd];
 
 * You may use InMobi class for additional targetting options.
 */

#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>
#import "IMNativeDelegate.h"
@interface IMNative : NSObject
/**
 * Publisher specific ad content in the format specified by the publisher. 
 * Use this content to render the ad.
 */
@property (atomic, strong) NSString* content;
/**
 * Native delegate to get success/failure response for the native ad.
 */
@property (nonatomic, assign) id<IMNativeDelegate> delegate;

/**
 * Initializes the native ad instance with the appId provided
 * @param appId Application Id registered on the InMobi portal.
 */
-(id)initWithAppId:(NSString*)appId;
/**
 * This method loads a native ad. This will automatically detach any previously attached view and invalidate the native instance.
 * Until the load is complete, no operations can be performed on this instance.
 */
-(void)loadAd;
/**
 * Once the rendering is done, this method attaches the native ad to the provided view.
 * The view passed to this method is the view that renders the ad from the native ad instance's content.
 * It is necessary to attach a view before any clicks can be reported to the native ad instance.
 * @param view The view rendered from content of the native ad.
 */
-(void)attachToView:(UIView*)view;
/**
 * This method detaches the native ad instance from the view it was attached to.
 * Once detached, The native instance gets invalidated until it is loaded again.
 * Only detach the view when you are completely done with it, like moving to a different page, not showing the ad anymore or
 * releasing the view that rendered the ad etc.
 */
-(void)detachFromView;
/**
 * This method reports a click to the native ad.
 * When the publisher considers an action on the view as a click,
 * call this method to report the click to InMobi.
 * @param params Any additional params to be passed for better reporting.
 */
-(void)handleClick:(NSDictionary*)params;

/**
 * A free form NSDictionary for any demographic information,
 * not available via InMobi class.
 */
@property (nonatomic,retain) NSDictionary *additionaParameters;
/**
 * A free form set of keywords, separated by ','
 * E.g: "sports,cars,bikes"
 */
@property (nonatomic,copy) NSString *keywords;
/**
 * Ref-tag key to be passed to an ad instance.
 */
@property (nonatomic,copy) NSString *refTagKey;
/**
 * Ref-tag value to be passed to an ad instance.
 */
@property (nonatomic,copy) NSString *refTagValue;

@end
