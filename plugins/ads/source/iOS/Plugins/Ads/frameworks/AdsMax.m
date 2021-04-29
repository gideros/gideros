//
//  AdsAdmob.m
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//
#include "gideros.h"
#import "AdsMax.h"
#import "AdsClass.h"

@implementation AdsMax
-(id)init{
    [ALSdk shared].mediationProvider = @"max";
        
    [[ALSdk shared] initializeSdkWithCompletionHandler:^(ALSdkConfiguration *configuration) {
        // Start loading ads
    }];
    
    self.appKey = @"";
    self.curType = @"";
    self.view_ = nil;
    self.mngr = [[AdsManager alloc] init];
    self.units=[[NSMutableDictionary alloc] init];
    self.listeners=[[NSMutableDictionary alloc] init];
    return self;
}

-(void)destroy{
    [self.mngr destroy];
    self.mngr = nil;
}

-(void)setKey:(NSMutableArray*)parameters{
    self.appKey = [parameters objectAtIndex:0];
}

-(void)loadAd:(NSMutableArray*)parameters{
    NSString *type = [parameters objectAtIndex:0];
    NSString *aid = [parameters objectAtIndex:1];
    ALSdk* sdk = [ALSdk sharedWithKey: self.appKey];
    if ([type isEqualToString:@"interstitial"]) {
        MAInterstitialAd *interstitial=[self.units objectForKey:aid];
        if (interstitial==nil) {
            AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
            AdsMaxListener *list = [[AdsMaxListener alloc] init:nil with:self];
            interstitial=[[MAInterstitialAd alloc] initWithAdUnitIdentifier:aid sdk:sdk];
            [listener setShow:^(){
                [AdsClass adDisplayed:[self class] forType:type];
                [interstitial showAd];
            }];
            [listener setDestroy:^(){
                [self hideAd:type];
            }];
            [listener setHide:^(){
            }];
            [self.mngr set:nil forType:type withListener:listener];
            [list setType:[self.mngr getState:type] with:self];
            interstitial.delegate=list;
            [self.units setObject:interstitial forKey:aid];
            [self.listeners setObject:list forKey:aid];
        }
        [interstitial loadAd];
    }
    else if ([type isEqualToString:@"v4vc"]) {
        MARewardedInterstitialAd *interstitial=[self.units objectForKey:aid];
        if (interstitial==nil) {
             AdsMaxListener *list = [[AdsMaxListener alloc] init:nil with:self];
            AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
            interstitial=[[MARewardedInterstitialAd alloc] initWithAdUnitIdentifier:aid sdk:sdk];
            [listener setShow:^(){
                [AdsClass adDisplayed:[self class] forType:type];
                [interstitial showAd];
            }];
            [listener setDestroy:^(){
                [self hideAd:type];
            }];
            [listener setHide:^(){
            }];
            [self.mngr set:nil forType:type withListener:listener];
            [list setType:[self.mngr getState:type] with:self];
            interstitial.delegate=list;
            [self.units setObject:interstitial forKey:aid];
            [self.listeners setObject:list forKey:aid];
        }
        [interstitial loadAd];
    }
#if TARGET_OS_TV==0
    else
    {
            if([self.mngr get:type] == nil)
            {
                self.curType = [type copy];
                float screenWidth = 0;
                CGRect screenRect = [[UIScreen mainScreen] bounds];
                if ([AdsClass isPortrait])
                {
                    screenWidth = screenRect.size.width;
                }
                else
                {
                    screenWidth = screenRect.size.height;
                }

                CGRect frame = CGRectMake(0, 0, screenWidth, 50);
                
                /* 320x50 is the standard banner size, though our banners will work
                 at wider sizes - so feel free to pin it with autolayout to the leading/trailing
                 space of its parent view. */
                self.view_=[[MAAdView alloc] initWithAdUnitIdentifier:aid sdk:sdk];
                self.view_.frame=frame;
                
                AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
                [listener setShow:^(){
                    [AdsClass adDisplayed:[self class] forType:type];
                    UIViewController *root = [AdsClass getRootViewController];
                    [root.view addSubview:self.view_];
                }];
                [listener setDestroy:^(){
                    [self hideAd:type];
                    if(self.view_ != nil)
                    {
                        self.view_.delegate = nil;
                        self.view_=nil;
                    }
                }];
                [listener setHide:^(){
                    if(self.view_ != nil)
                    {
                        [self.view_ removeFromSuperview];
                        [AdsClass adDismissed:[self class] forType:type];
                    }

                }];
                [self.mngr set:self.view_ forType:type withListener:listener];
                AdsMaxListener *list =[[AdsMaxListener alloc] init:[self.mngr getState:type] with:self];
                
                self.view_.delegate = list;
                [self.mngr setAutoKill:false forType:type];
                [self.view_ loadAd];
        }
        else
        {
            [AdsClass adError:[self class] with:[NSString stringWithFormat:@"Unknown type: %@", type]];
        }
    }
#endif
}

-(void)showAd:(NSMutableArray*)parameters{
    NSString *type = [parameters objectAtIndex:0];
    if([self.mngr get:type] == nil)
        [self loadAd:parameters];
    [self.mngr show:type];
}

-(void)hideAd:(NSString*)type{
    [self.mngr hide:type];
}

-(void)enableTesting{
}

-(UIView*)getView{
    return (UIView*)[self.mngr get:self.curType];
}

@end


@implementation AdsMaxListener

-(id)init:(AdsState*)state with:(AdsMax*)instance{
    self.state = state;
    self.instance = instance;
    return self;
}

-(void)setType:(AdsState*)state with:(AdsMax*)instance{
    self.state = state;
    self.instance = instance;
}

/**
 * This method is called when a new ad has been loaded.
 */
- (void)didLoadAd:(MAAd *)ad
{
    if([[self.state getType] isEqual:@"interstitial"] || [[self.state getType] isEqual:@"v4vc"]){
        [self.state setObject:ad];
    }
    [AdsClass adReceived:[self.instance class] forType:[self.state getType]];
    [self.state load];
}

/**
 * This method is called when an ad could not be retrieved.
 *
 * Common error codes:
 * 204 - no ad is available
 * 5xx - internal server error
 * negative number - internal errors
 *
 * @param adUnitIdentifier  Ad unit identifier for which the ad was requested.
 * @param errorCode         An error code representing the failure reason. Common error codes are defined in `MAErrorCode.h`.
 */
- (void)didFailToLoadAdForAdUnitIdentifier:(NSString *)adUnitIdentifier withErrorCode:(NSInteger)code
{
    NSString *error = @"Unknown error";
    if(code == kMAErrorCodeNoFill)
    {
        error = @"No fill";
    }
    else if(code == kMAErrorCodeMediationAdapterLoadFailed){
        error = @"Load failed";
    }
    else if(code == kMAErrorCodeUnspecifiedError){
        error = @"Unspecified error";
    }
    else if(code == kMAErrorCodeInvalidInternalState){
        error = @"Invalid internal state";
    }
    else if(code == kMAErrorCodeFullscreenAdAlreadyShowing){
        error = @"Fullscreen Ad already showing";
    }

    [AdsClass adFailed:[self.instance class] with:error forType:[self.state getType]];
    [self.state reset];
}

/**
 * This method is invoked when the ad failed to displayed.
 *
 * This method is invoked on the main UI thread.
 *
 * @param ad        Ad that was just failed to display.
 * @param errorCode Error that indicates display failure. Common error codes are defined in `MAErrorCode.h`.
 */
- (void)didFailToDisplayAd:(MAAd *)ad withErrorCode:(NSInteger)code
{
    NSString *error = @"Unknown error";
    if(code == kMAErrorCodeNoFill)
    {
        error = @"No fill";
    }
    else if(code == kMAErrorCodeMediationAdapterLoadFailed){
        error = @"Load failed";
    }
    else if(code == kMAErrorCodeUnspecifiedError){
        error = @"Unspecified error";
    }
    else if(code == kMAErrorCodeInvalidInternalState){
        error = @"Invalid internal state";
    }
    else if(code == kMAErrorCodeFullscreenAdAlreadyShowing){
        error = @"Fullscreen Ad already showing";
    }
    [AdsClass adFailed:[self.instance class] with:error forType:[self.state getType]];
    [self.state reset];
}
/**
 * This method is invoked when an ad is displayed.
 *
 * This method is invoked on the main UI thread.
 */
- (void)didDisplayAd:(MAAd *)ad
{
}

/**
 * This method is invoked when the ad is clicked.
 *
 * This method is invoked on the main UI thread.
 */
- (void)didClickAd:(MAAd *)ad
{
    [AdsClass adActionBegin:[self.instance class] forType:[self.state getType]];
}
/**
 * This method is invoked when an ad is hidden.
 *
 * This method is invoked on the main UI thread.
 */
- (void)didHideAd:(MAAd *)ad
{
    [AdsClass adDismissed:[self.instance class] forType:[self.state getType]];
}

/**
 * This method will be invoked when rewarded video has started.
 */
- (void)didStartRewardedVideoForAd:(MAAd *)ad
{
    [AdsClass adActionBegin:[self.instance class] forType:[self.state getType]];
}
/**
 * This method will be invoked when rewarded video has completed.
 */
- (void)didCompleteRewardedVideoForAd:(MAAd *)ad
{
  [AdsClass adActionEnd:[self.instance class] forType:[self.state getType]];
}

/**
 * This method will be invoked when a user should be granted a reward.
 *
 * @param ad     Ad for which reward ad was rewarded for.
 * @param reward The reward to be granted to the user.
 */
- (void)didRewardUserForAd:(MAAd *)ad withReward:(MAReward *)reward;
{
    [AdsClass adActionEnd:[self.instance class] forType:[self.state getType]];
    [AdsClass adRewarded:[self.instance class] forType:[self.state getType] withAmount:(int)reward.amount];
}

/**
 * This method will be invoked when the `MAAdView` has expanded the full screen.
 *
 * @param ad An ad for which the ad view expanded for.
 */
- (void)didExpandAd:(MAAd *)ad
{
    
}

/**
 * This method will be invoked when the `MAAdView` has collapsed back to its original size.
 *
 * @param ad An ad for which the ad view collapsed for.
 */
- (void)didCollapseAd:(MAAd *)ad
{
    
}

@end

