//
//  ALIncentivizedInterstitialAd.h
//  sdk
//
//  Created by Matt Szaro on 10/1/13.
//
//

#import <Foundation/Foundation.h>
#import "ALNullabilityAnnotations.h"

#import "ALInterstitialAd.h"
#import "ALAdVideoPlaybackDelegate.h"
#import "ALAdDisplayDelegate.h"
#import "ALAdLoadDelegate.h"
#import "ALAdRewardDelegate.h"

/**
 *  This class is used to show rewarded videos to the user. These differ from regular interstitials in that they allow you to provide you user virtual currency in exchange for watching a video.
 */

@interface ALIncentivizedInterstitialAd : NSObject

/**
 * @name Ad Delegates
 */

/**
 *  An object conforming to the ALAdDisplayDelegate protocol, which, if set, will be notified of ad show/hide events.
 */
@property (strong, nonatomic) id<ALAdDisplayDelegate> __alnullable adDisplayDelegate;

/**
 *  An object conforming to the ALAdVideoPlaybackDelegate protocol, which, if set, will be notified of video start/stop events.
 */
@property (strong, nonatomic) id<ALAdVideoPlaybackDelegate> __alnullable adVideoPlaybackDelegate;

/**
 * @name Integration, Class Methods
 */

/**
 * Get a reference to the shared instance of ALIncentivizedInterstitialAd.
 *
 * This wraps the [ALSdk shared] call, and will only work if you hve set your SDK key in Info.plist.
*/
+(alnonnull ALIncentivizedInterstitialAd*) shared;

/**
 * Pre-load an incentivized interstitial, and notify your provided Ad Load Delegate.
 *
 * Invoke once to preload, then do not invoke again until the ad has has been closed (e.g., ALAdDisplayDelegate's adWasHidden callback).
 * You may pass a nil argument to preloadAndNotify if you intend to use the synchronous ( isIncentivizedAdReady ) flow. Note that this is NOT recommended; we HIGHLY RECOMMEND you use an ad load delegate.
 * This method uses the shared instance, and will only work if you have set your SDK key in Info.plist.
 * Note that we internally try to pull down the next ad's resources before you need it. Therefore, this method may complete immediately in many circumstances.
 *
 * @param adLoadDelegate The delegate to notify that preloading was completed. May be nil.
 */
+(void) preloadAndNotify: (alnullable id<ALAdLoadDelegate>) adLoadDelegate;

/**
 * Check if an ad is currently ready on this object. You must call preloadAndNotify in order to reach this state.
 *
 * It is highly recommended that you implement an asynchronous flow (using an ALAdLoadDelegate with preloadAndNotify) rather than checking this property. This class does not contain a queue and can hold only one preloaded ad at a time. Therefore, you should NOT simply call preloadAndNotify: any time this method returns NO; it is important to invoke only one ad load - then not invoke any further loads until the ad has been closed (e.g., ALAdDisplayDelegate's adWasHidden callback).
 *
 * @return YES if an ad has been loaded into this incentivized interstitial and is ready to display. NO otherwise.
 */
+(BOOL) isReadyForDisplay;

/**
 * Show an incentivized interstitial over the current key window, using the most recently pre-loaded ad.
 *
 * You must call preloadAndNotify before calling showOver.
 */
+(void) show;

/**
 * Show an incentivized interstitial over the current key window, using the most recently pre-loaded ad.
 *
 * You must call preloadAndNotify before calling showOver.
 *
 * Using the ALAdRewardDelegate, you will be able to verify with AppLovin servers the the video view is legitimate,
 * as we will confirm whether the specific ad was actually served - then we will ping your server with a url for you to update
 * the user's balance. The Reward Validation Delegate will tell you whether we were able to reach our servers or not. If you receive
 * a successful response, you should refresh the user's balance from your server. For more info, see the documentation.
 *
 * @param adRewardDelegate The reward delegate to notify upon validating reward authenticity with AppLovin.
 *
 */
+(void) showAndNotify: (alnullable id<ALAdRewardDelegate>) adRewardDelegate;

/**
 * Show an incentivized interstitial, using the most recently pre-loaded ad.
 *
 * You must call preloadAndNotify before calling showOver.
 *
 * Using the ALAdRewardDelegate, you will be able to verify with AppLovin servers the the video view is legitimate,
 * as we will confirm whether the specific ad was actually served - then we will ping your server with a url for you to update
 * the user's balance. The Reward Validation Delegate will tell you whether we were able to reach our servers or not. If you receive
 * a successful response, you should refresh the user's balance from your server. For more info, see the documentation.
 *
 * @param adRewardDelegate The reward delegate to notify upon validating reward authenticity with AppLovin.
 * @param window The UIWindow over which the rewarded video should be displayed.
 *
 */
+(void) showOver: (alnonnull UIWindow*) window andNotify: (alnullable id<ALAdRewardDelegate>) adRewardDelegate;

/**
 * @name Integration, Instance Methods
 */

/**
 * Initialize an incentivized interstitial with a specific custom SDK.
 *
 * This is necessary if you use <code>[ALSdk sharedWithKey: ...]</code>.
 *
 * @param anSdk An SDK instance to use.
 */
-(alnonnull instancetype) initWithSdk: (alnonnull ALSdk*) anSdk;
-(alnonnull instancetype) initIncentivizedInterstitialWithSdk: (alnonnull ALSdk*) anSdk __deprecated_msg("Use initWithSdk instead.");

/**
 * Pre-load an incentivized interstitial, and notify your provided Ad Load Delegate.
 *
 * Invoke once to preload, then do not invoke again until the ad has has been closed (e.g., ALAdDisplayDelegate's adWasHidden callback).
 * You may pass a nil argument to preloadAndNotify if you intend to use the synchronous ( isIncentivizedAdReady ) flow. Note that this is NOT recommended; we HIGHLY RECOMMEND you use an ad load delegate.
 * Note that we internally try to pull down the next ad's resources before you need it. Therefore, this method may complete immediately in many circumstances.
 *
 * @param adLoadDelegate The delegate to notify that preloading was completed.
 */
-(void) preloadAndNotify: (alnullable id<ALAdLoadDelegate>) adLoadDelegate;

/**
 * Check if an ad is currently ready on this object. You must call preloadAndNotify in order to reach this state.
 *
 * It is highly recommended that you implement an asynchronous flow (using an ALAdLoadDelegate with preloadAndNotify) rather than checking this property. This class does not contain a queue and can hold only one preloaded ad at a time. Therefore, you should NOT simply call preloadAndNotify: any time this method returns NO; it is important to invoke only one ad load - then not invoke any further loads until the ad has been closed (e.g., ALAdDisplayDelegate's adWasHidden callback).
 *
 * @return YES if an ad has been loaded into this incentivized interstitial and is ready to display. NO otherwise.
 */
@property (readonly, atomic, getter=isReadyForDisplay) BOOL readyForDisplay;

/**
 * Show an incentivized interstitial over the current key window, using the most recently pre-loaded ad.
 *
 * You must call preloadAndNotify before calling showOver.
 */
-(void) show;

/**
 * Show an incentivized interstitial over the current key window, using the most recently pre-loaded ad.
 *
 * You must call preloadAndNotify before calling showOver.
 *
 * Using the ALAdRewardDelegate, you will be able to verify with AppLovin servers the the video view is legitimate,
 * as we will confirm whether the specific ad was actually served - then we will ping your server with a url for you to update
 * the user's balance. The Reward Validation Delegate will tell you whether we were able to reach our servers or not. If you receive
 * a successful response, you should refresh the user's balance from your server. For more info, see the documentation.
 *
 * @param adRewardDelegate The reward delegate to notify upon validating reward authenticity with AppLovin.
 *
 */
-(void) showAndNotify: (alnullable id<ALAdRewardDelegate>) adRewardDelegate;

/**
 * Show an incentivized interstitial, using the most recently pre-loaded ad.
 *
 * You must call preloadAndNotify before calling showOver.
 *
 * Using the ALAdRewardDelegate, you will be able to verify with AppLovin servers that the video view is legitimate,
 * as we will confirm whether the specific ad was actually served - then we will ping your server with a url for you to update
 * the user's balance. The Reward Validation Delegate will tell you whether we were able to reach our servers or not. If you receive
 * a successful response, you should refresh the user's balance from your server. For more info, see the documentation.
 *
 * @param adRewardDelegate The reward delegate to notify upon validating reward authenticity with AppLovin.
 * @param window The UIWindow over which the rewarded video should be displayed.
 *
 */
-(void) showOver: (alnonnull UIWindow*) window andNotify: (alnullable id<ALAdRewardDelegate>) adRewardDelegate;

/**
 * Dismiss an incentivized interstitial prematurely, before video playback has completed.
 *
 * In most circumstances, this is not recommended, as it may confuse users by denying them a reward.
 */
-(void) dismiss;

/**
 * @name Custom User Identifiers
 */

/**
 * Set a string which identifies the current user, which will be passed through to your server via our optional S2S postbacks.
 *
 * If you're using reward validation, you can optionally set a user identifier to be included with
 * currency validation postbacks. For example, a user name. We'll include this in the postback when we
 * ping your currency endpoint from our server.
 *
 * @param userIdentifier Some descriptive string identifying the user, usually a username.
 */
+(void) setUserIdentifier: (alnullable NSString*) userIdentifier;

/**
 *  Get the currently set user identification string.
 *
 *  @return The last value supplied via setUserIdentifier:
 */
+(alnullable NSString*) userIdentifier;

- (alnullable id) init __attribute__((unavailable("Use [ALIncentivizedInterstitialAd shared] or initWithSdk: instead.")));
@end
