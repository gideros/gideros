//
//  IMInterstitial.h
//  InMobi Monetization SDK
//
//  Copyright (c) 2013 InMobi. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "InMobi.h"
#import "IMInterstitialDelegate.h"
@protocol IMNetworkExtras;

/**
 * Class to display an Interstitial Ad.
 * Interstitials are full screen advertisements that are shown at natural
 * transition points in your application such as between game levels, when
 * switching news stories, in general when transitioning from one view controller
 * to another. It is best to request for an interstitial several seconds before
 * when it is actually needed, so that it can preload its content and become
 * ready to present, and when the time comes, it can be immediately presented to
 * the user with a smooth experience.
 */
@interface IMInterstitial : NSObject

#pragma mark - Initialization
/**
 * Initializes an IMInterstitial instance with the specified appId.
 * @param appId Application Id registered on the InMobi portal.
 * @note |appId| is required to be non empty, else the instance of
 * IMInterstitial is not created and nil is returned instead.
 */
- (id)initWithAppId:(NSString *)appId;
/**
 * Initializes an IMInterstitial instance, and sets the specified slot id.
 * @param slotId Slot id to uniquely identify a placement in your app.
 */
- (id)initWithSlotId:(long long)slotId;

#pragma mark - Initialization for IB Integration
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

#pragma mark - Loading Interstitial
/**
 * Loads the Interstitial. Additional targetting options may be provided using
 * the InMobi class.
 * It is best to call this method several seconds before the interstitial is
 * needed to preload its content. Once, it is fetched and is ready to display,
 * you can present it using presentInterstitialAnimated: method.
 */
- (void)loadInterstitial;
/**
 * Stops the current loading of the Interstitial if in progress.
 */
- (void)stopLoading;

#pragma mark - Presenting Interstitial
/**
 * This presents the interstitial ad that takes over the entire screen until
 * the user dismisses it. This has no effect unless the interstitial state is
 * kIMInterstitialStateReady and/or the delegate's interstitialDidReceiveAd:
 * has been received. After the interstitial has been dismissed by the user,
 * the delegate's interstitialDidDismissScreen: will be called.
 * @param animated Display the interstitial by using an animation. This is
 * similar to presenting a Modal-View-Controller like animation, from the bottom.
 */
- (void)presentInterstitialAnimated:(BOOL)animated;

#pragma mark - Optional properties
/**
 * Delegate object that receives state change notifications from this
 * interstitial object. Typically, this is a UIViewController instance.
 * @warning Whenever you release the Interstitial object, make sure to set its
 * delegate to nil to prevent any chance of your application crashing.
 */
@property (nonatomic, assign) NSObject<IMInterstitialDelegate> *delegate;
/**
 * Returns the state of the interstitial ad. The delegate's
 * interstitialDidFinishRequest: will be called when this switches from the
 * kIMInterstitialStateInit state to the kIMInterstitialStateReady state.
 */
@property (readonly) IMInterstitialState state;
/**
 * Set the adMode property to switch to app gallery mode.
 * The default value is network.
 */
@property (nonatomic) IMAdMode adMode;

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

@end
