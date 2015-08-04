//
//  ALAdView.h
//  sdk
//
//  Created by Basil on 3/1/12.
//  Copyright (c) 2013, AppLovin Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "ALSdk.h"
#import "ALAdService.h"

/**
 *  This <code>UIView</code> subclass is used to render and show ads.
 */
@interface ALAdView : UIView<ALAdLoadDelegate>

/**
 * @name Ad Delegates
 */

/**
 *  An object conforming to the ALAdLoadDelegate protocol, which, if set, will be notified of ad load events.
 */
@property (strong, atomic) id<ALAdLoadDelegate> __alnullable adLoadDelegate;

/**
 *  An object conforming to the ALAdDisplayDelegate protocol, which, if set, will be notified of ad show/hide events.
 */
@property (strong, atomic) id<ALAdDisplayDelegate> __alnullable adDisplayDelegate;

// Primarily for internal use; banners and mrecs cannot contain videos.
@property (strong, atomic) id<ALAdVideoPlaybackDelegate> __alnullable adVideoPlaybackDelegate;

/**
 * @name Ad View Configuration
 */

/**
 *  The size of ads to be loaded within this ALAdView.
 */
@property (strong, atomic) ALAdSize* __alnonnull adSize;

/**
 *  Whether or not this ALAdView should automatically load and rotate banners.
 *
 * If YES, ads will be automatically loaded and updated. If NO, you are reponsible for this behavior via [ALAdView loadNextAd]. Defaults to YES.
 */
@property (assign, atomic, getter=isAutoloadEnabled, setter=setAutoloadEnabled:) BOOL autoload;
@property (assign, atomic, getter=isAutoloadEnabled, setter=setAutoloadEnabled:) BOOL shouldAutoload __deprecated_msg("Use 'autoload' property intead.");

/**
 *  The UIViewController in whose view this ALAdView is placed.
 */
@property (strong, atomic) UIViewController* __alnullable parentController;

/**
 * @name Loading and Rendering Ads
 */

/**
 * Start loading a new advertisement. This method will return immediately. An
 * advertisement will be rendered by this view asynchonously when available.
 */
-(void) loadNextAd;

/**
 * Check if the next ad is currently ready to display.
 *
 * @return YES if a subsequent call to a show method will result in an immediate display. NO if a call to a show method will require network activity first.
 */
@property (readonly, atomic, getter=isReadyForDisplay) BOOL readyForDisplay;

/**
 * Render a specific ad that was loaded via ALAdService.
 *
 * @param ad Ad to render. Must not be nil.
 */
-(void) render: (alnullable ALAd *) ad;

/**
 * @name Initialization
 */

/**
 *  Initialize the ad view with a given size.
 *
 *  @param aSize ALAdSize representing the size of this ad. For example, [ALAdSize sizeBanner].
 *
 *  @return A new instance of ALAdView.
 */
-(alnonnull instancetype) initWithSize: (alnonnull ALAdSize*) aSize;

/**
 *  Initialize the ad view with a given size.
 *
 *  @param anSdk Instance of ALSdk to use.
 *  @param aSize ALAdSize representing the size of this ad. For example, [ALAdSize sizeBanner].
 *
 *  @return A new instance of ALAdView.
 */
-(alnonnull instancetype) initWithSdk: (alnonnull ALSdk*) anSdk size: (alnonnull ALAdSize*) aSize;

/**
 * Initialize ad view with a given frame, ad size, and ALSdk instance.
 *
 * @param aFrame  Frame to use.
 * @param aSize   Ad size to use.
 * @param anSdk   Instace of ALSdk to use.
 *
 * @return A new instance of ALAdView.
 */
- (alnonnull id) initWithFrame: (CGRect) aFrame size: (alnonnull ALAdSize*) aSize sdk: (alnonnull ALSdk*) anSdk;

/*
 * @name Deprecated Methods
 */

/*
 * Initialize ad view as a <code>BANNER</code>.
 *
 * @return A new instance of ALAdView.
 *
 * @deprecated Use initWithSize: instead.
 * @warning Deprecated. Instead, use <code>[adView initWithSize: [ALAdSize sizeBanner]]</code>.
 */
-(alnonnull instancetype) initBannerAd __deprecated_msg("Use initWithSize: [ALAdSize sizeBanner] instead.");

/*
 * Initialize ad view as an <code>MREC</code>.
 *
 * @return A new instance of ALAdView.
 *
 * @warning Deprecated. Instead, use <code>[adView initWithSize: [ALAdSize sizeMrec]]</code>.
 */
-(alnonnull instancetype) initMRecAd __deprecated_msg("Use initWithSize: [ALAdSize sizeMrec] instead.");

/*
 * Initialize ad view as a <code>BANNER</code> with a specified ALSdk instance.
 *
 * @param anSdk Instace of AppLovin SDK to use.
 *
 * @return A new instance of ALAdView.
 *
 * @warning Deprecated. Instead, use <code>[adView initWithSdk: sdk size: [ALAdSize sizeBanner]]</code>.
 */
-(alnonnull instancetype) initBannerAdWithSdk: (alnonnull ALSdk *) anSdk __deprecated_msg("Use initWithSize: [ALAdSize sizeBanner] instead.");

/*
 * Initialize ad view as an <code>MREC</code> with a specified ALSdk instance.
 *
 * @param anSdk Instance of AppLovin SDK to use.
 *
 * @return A new instance of ALAdView.
 *
 * @warning Deprecated. Instead, use <code>[adView initWithSdk: sdk size: [ALAdSize sizeMrec]]</code>.
 */
-(alnonnull instancetype) initMRecAdWithSdk: (alnonnull ALSdk *) anSdk __deprecated_msg("Use initWithSize: [ALAdSize sizeMrec] instead.");


- (alnullable id)init __attribute__((unavailable("Use an init method with a size instead - for example, initBannerAd.")));
@end
