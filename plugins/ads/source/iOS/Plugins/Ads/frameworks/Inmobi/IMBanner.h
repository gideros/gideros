//
//  IMBanner.h
//  InMobi Monetization SDK
//
//  Copyright (c) 2013 InMobi. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "InMobi.h"
#import "IMBannerDelegate.h"

#define REFRESH_INTERVAL_OFF  (-1)

#pragma mark - Ad Sizes
/**
 * The ad size equivalent to CGSizeMake(320, 48).
 * @deprecated Will be removed in a future release. Use IM_UNIT_320x50 instead.
 */
#define IM_UNIT_320x48        9
/**
 * Medium Rectangle size for the iPad (especially in a UISplitView's left pane).
 * The ad size equivalent to CGSizeMake(300, 250).
 */
#define IM_UNIT_300x250       10
/**
 * Leaderboard size for the iPad.
 * The ad size equivalent to CGSizeMake(728,90).
 */
#define IM_UNIT_728x90        11
/**
 * Full Banner size for the iPad (especially in a UIPopoverController or in
 * UIModalPresentationFormSheet).
 * The ad size equivalent to CGSizeMake(468x60).
 */
#define IM_UNIT_468x60        12
/**
 * Skyscraper size, designed for iPad's screen size.
 * The ad size equivalent to CGSizeMake(120x600).
 */
#define IM_UNIT_120x600       13
/**
 * Default ad size for iPhone and iPod Touch.
 * The ad size equivalent to CGSizeMake(320, 48).
 */
#define IM_UNIT_320x50        15

/**
 This is a UIView class that displays banner ads. A minimum implementation to
 get an ad is:

   - Initialize an IMBanner instance either by providing appid and adsize or
     by providing slot id.
   - Load the Banner.
   - Place the banner on the screen.

 Below is a sample example:
    banner = [[IMBanner alloc] initWithFrame:CGRectMake(0,0,320,50)
                                       appId:@"YOUR_APP_ID"
                                      adSize:IM_UNIT_320x50];
    banner.delegate = self;
    [banner loadBanner];
 
 You may use InMobi class for additional targetting options.
 */
@interface IMBanner : UIView

#pragma mark - Initialization
/**
 * Initializes an IMBanner instance with the specified appId and adSize.
 * @param frame CGRect for this view, typically according to the requested size.
 * @param appId Application Id registered on the InMobi portal.
 * @param adSize Ad size id to request the specific banner size.
 * @note |appId| is required to be non empty, else the instance of IMBanner is
 * not created and nil is returned instead.
 */
- (id)initWithFrame:(CGRect)frame appId:(NSString *)appId adSize:(int)adSize;
/**
 * Initializes an IMBanner instance with the specified slot id.
 * @param frame CGRect for this view, typically according to the requested size.
 * @param slotId Slot id to uniquely identify a placement in your app.
 */
- (id)initWithFrame:(CGRect)frame slotId:(long long)slotId;

#pragma mark - Initialization for IB Integration
/**
 * Ad size id for the request.
 * @note Check the ad size macros for the available values of ad-size.
 */
@property (nonatomic) int adSize;
/**
 * Application Id for the request.
 * @note Use the application id registered on the InMobi portal.
 */
@property (nonatomic, copy) NSString *appId;
/**
 * Slot id is the specific placement id for the request.
 * @note Use the slot id registered on the InMobi portal.
 */
@property (nonatomic) long long slotId;

#pragma mark - Loading Banner
/**
 * Loads the Banner. Additional targetting options may be provided using the
 * InMobi class.
 */
- (void)loadBanner;
/**
 * Stops the current loading of the Banner if in progress.
 */
- (void)stopLoading;

#pragma mark - Optional properties
/**
 * Delegate object that receives state change notifications from this view.
 * Typically, this is a UIViewController instance.
 * @note Whenever you're releasing the Banner, make sure you set its delegate
 * to nil and remove it from its superview to prevent any chance of your
 * application crashing.
 */
@property (nonatomic, assign) IBOutlet NSObject<IMBannerDelegate> *delegate;
/**
 * Starts or stops the auto refresh of ads.
 * The refresh interval is measured between the completion(success or failure)
 * of the previous ad request and start of the next ad request. By default,
 * the refresh interval is set to 60 seconds. Setting a new valid refresh
 * interval value will start the auto refresh of ads if not already started.
 * Use REFRESH_INTERVAL_OFF as the parameter to switch off auto refresh.
 * When auto refresh is turned off, use the loadBanner method to manually load
 * new ads. The SDK will not refresh ads if the screen is in the background or
 * if the phone is locked.
 */
@property (nonatomic) int refreshInterval;
/**
 * The animation transition, when a banner ad is refresh.
 * @note: Applicable for banner ads only.
 */
@property (nonatomic,assign) UIViewAnimationTransition refreshAnimation;

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
