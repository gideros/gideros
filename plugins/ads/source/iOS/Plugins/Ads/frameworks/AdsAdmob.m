//
//  AdsAdmob.m
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//
#include "gideros.h"
#import "AdsAdmob.h"
#import "AdsClass.h"
#include <UserMessagingPlatform/UserMessagingPlatform.h>


@implementation AdsAdmob
-(id)init{
    self.testID = @"";
    self.currentType = GADAdSizeBanner;
    self.currentSize = @"banner";
    self.appKey = @"";
     self.mngr = [[AdsManager alloc] init];
    
    [GADMobileAds.sharedInstance startWithCompletionHandler:nil];
    return self;
}

- (const GADAdSize) getAdType:(NSString*)type orientation:(NSString*)orientation{
    if([type isEqual:@"banner"])
    {
        return GADAdSizeBanner;
    }
    else if([type isEqual:@"iab_mrect"])
    {
        return GADAdSizeMediumRectangle;
    }
    else if([type isEqual:@"iab_banner"])
    {
        return GADAdSizeFullBanner;
    }
    else if([type isEqual:@"iab_leaderboard"])
    {
        return GADAdSizeLeaderboard;
    }
    else if([type isEqual:@"smart_banner"] || [type isEqual:@"auto"])
    {
        /*
        if ([AdsClass isPortrait])
        {
            return &kGADAdSizeSmartBannerPortrait;
        }
        else
        {
            return &kGADAdSizeSmartBannerLandscape;
        }
        */
        CGSize vsize=[AdsClass getRootViewController].view.bounds.size;
        if (orientation == nil){
            if ([AdsClass isPortrait])
            {
                return GADPortraitAnchoredAdaptiveBannerAdSizeWithWidth(vsize.width);
            }
            else
            {
                return GADLandscapeAnchoredAdaptiveBannerAdSizeWithWidth(vsize.width);
            }
        }
        else{
            if ([orientation isEqual:@"landscapeLeft"] || [orientation isEqual:@"landscapeRight"])
                return GADLandscapeAnchoredAdaptiveBannerAdSizeWithWidth(vsize.width);
            else
                return GADPortraitAnchoredAdaptiveBannerAdSizeWithWidth(vsize.width);
        }
    }
    return GADAdSizeBanner;
}

-(void)destroy{
    [self.mngr destroy];
     self.mngr = nil;
    
    self.currentSize = nil;
    self.appKey = nil;
    self.testID = nil;
}

-(void)setKey:(NSMutableArray*)parameters{
    self.appKey = [parameters objectAtIndex:0];  //the first is banner id
    
    if ([parameters count] >=3)
    {
        //2 is app id (unused), 3 is test identifier
        GADMobileAds.sharedInstance.requestConfiguration.testDeviceIdentifiers = @[ [parameters objectAtIndex:2] ];
       /* [GADMobileAds configureWithApplicationID:[parameters objectAtIndex:1]]; */
    }
    if ([parameters count] >=4)
        self.testID=[parameters objectAtIndex:3];
}

-(void)loadAd:(NSMutableArray*)parameters{
    NSString *type = [parameters objectAtIndex:0];
    NSString *placeId = nil;
    if([parameters count] > 1)
    {
        placeId = [parameters objectAtIndex:1];
    }
    else
    {
        placeId = self.appKey;
    }
    if ([type isEqualToString:@"interstitial"]) {
        //release it after closed
        if([self.mngr get:type] != nil){
            [self.mngr reset:type];
        }

        AdsAdmobFullscreenListener *adlistener=[[AdsAdmobFullscreenListener alloc] init:[self.mngr getState:type] placeId:placeId ad:nil with:self];

        GADRequest *request = [GADRequest request];
        [GADInterstitialAd loadWithAdUnitID:placeId
                                    request:request
                          completionHandler:^(GADInterstitialAd *ad, NSError *error) {
          if (error) {
               [AdsClass adFailed:[self class] with:[error localizedDescription] forType:@"interstitial"];
              [self.mngr reset:@"interstitial"];
            return;
          }
            
            ad.fullScreenContentDelegate=adlistener;
            adlistener.ad=ad;
            
            [AdsClass adReceived:[self class] forType:@"interstitial"];
            [self.mngr load:@"interstitial"];
         }];
        
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
        [listener setShow:^(){
            AdsAdmobFullscreenListener* ls=(AdsAdmobFullscreenListener *)[self.mngr get:type];
            GADInterstitialAd* interstitial_=(GADInterstitialAd *)ls.ad;
            if (interstitial_){
                [interstitial_ presentFromRootViewController:[AdsClass getRootViewController]];
            }
        }];
        [listener setDestroy:^(){
            [self hideAd:type];
        }];
        [listener setHide:^(){
        }];
        [self.mngr set:adlistener forType:type withListener:listener];
        adlistener.state=[self.mngr getState:type];
        [self.mngr setPreload:true forType:type];
        
        //is it OK  to release the interstitial_ when it is showing the ads if set autokill to true?
        [self.mngr setAutoKill:false forType:type];
    }
    else if([type isEqualToString:@"rewarded"]){
        if([self.mngr get:type] != nil){
            [self.mngr reset:type];
        }

        AdsAdmobFullscreenListener *adlistener=[[AdsAdmobFullscreenListener alloc] init:[self.mngr getState:type] placeId:placeId ad:nil with:self];

        GADRequest *request = [GADRequest request];
        [GADRewardedAd loadWithAdUnitID:placeId
                                    request:request
                          completionHandler:^(GADRewardedAd *ad, NSError *error) {
			if (error) {
			  [AdsClass adFailed:[self class] with:[error localizedDescription] forType:type];
			  [self.mngr reset:type];
			  return;
			}
			    
			ad.fullScreenContentDelegate=adlistener;
			adlistener.ad=ad;
			[AdsClass adReceived:[self class] forType:type];
			[self.mngr load:type];
		}];
                        
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
        [listener setShow:^(){
            AdsAdmobFullscreenListener* ls=(AdsAdmobFullscreenListener *)[self.mngr get:type];
            GADRewardedAd* interstitial_=(GADRewardedAd *)ls.ad;
             if (interstitial_){
                [interstitial_ presentFromRootViewController:[AdsClass getRootViewController] userDidEarnRewardHandler:^{
                    
                    GADAdReward *reward =
                        interstitial_.adReward;
                    int amount = [reward.amount intValue];
                    [AdsClass adRewarded:[self class] forType:type withAmount:amount];
                  }];
                AdsState *state = [self.mngr getState:type];
                if(state != nil)
                    [state reset:false];
            }
        }];
        [listener setDestroy:^(){
            [self hideAd:type];           
        }];
        [listener setHide:^(){
        }];
    
        [self.mngr set:adlistener forType:type withListener:listener];
        adlistener.state=[self.mngr getState:type];
        [self.mngr setPreload:true forType:type];
        [self.mngr setAutoKill:false forType:type];
    }
    else if([type isEqualToString:@"rewarded_interstitial"]){
        if([self.mngr get:type] != nil){
            [self.mngr reset:type];
        }

        AdsAdmobFullscreenListener *adlistener=[[AdsAdmobFullscreenListener alloc] init:[self.mngr getState:type] placeId:placeId ad:nil with:self];
        GADRequest *request = [GADRequest request];
        [GADRewardedInterstitialAd loadWithAdUnitID:placeId
                                    request:request
                          completionHandler:^(GADRewardedInterstitialAd *ad, NSError *error) {
            if (error) {
              [AdsClass adFailed:[self class] with:[error localizedDescription] forType:type];
              [self.mngr reset:type];
              return;
            }
            
            ad.fullScreenContentDelegate=adlistener;
            adlistener.ad=ad;
            [AdsClass adReceived:[self class] forType:type];
            [self.mngr load:type];
         }];
         
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
        [listener setShow:^(){
            AdsAdmobFullscreenListener* ls=(AdsAdmobFullscreenListener *)[self.mngr get:type];
            GADRewardedInterstitialAd* interstitial_=(GADRewardedInterstitialAd *)ls.ad;
             if (interstitial_){
                [interstitial_ presentFromRootViewController:[AdsClass getRootViewController] userDidEarnRewardHandler:^{
                    
                    GADAdReward *reward =
                        interstitial_.adReward;
                    int amount = [reward.amount intValue];
                    [AdsClass adRewarded:[self class] forType:type withAmount:amount];
                  }];
                AdsState *state = [self.mngr getState:type];
                if(state != nil)
                    [state reset:false];
            }
        }];
        [listener setDestroy:^(){
            [self hideAd:type];           
        }];
        [listener setHide:^(){
        }];
    
        [self.mngr set:adlistener forType:type withListener:listener];
        adlistener.state=[self.mngr getState:type];
        [self.mngr setPreload:true forType:type];
        [self.mngr setAutoKill:false forType:type];
    }
    else
    {
        if([self.mngr get:type] == nil)
        {
            NSString *orientation = nil;
            if ([parameters count] > 2)
                orientation = [parameters objectAtIndex:2];
            
            self.currentType = [self getAdType:type orientation:orientation];
            
            GADBannerView *view_ = [[GADBannerView alloc] initWithAdSize:self.currentType];
            view_.adUnitID = placeId;
            view_.rootViewController = [AdsClass getRootViewController];
            
            AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
            [listener setShow:^(){
                self.currentSize = type;
                [AdsClass adDisplayed:[self class] forType:type];
                [view_.rootViewController.view addSubview:view_];
            }];
            [listener setDestroy:^(){
                [self hideAd:type];
                if(view_ != nil)
                {
                    view_.delegate = nil;
                    //view_=nil;
                }
            }];
            [listener setHide:^(){
                if(view_ != nil)
                {
                    [view_ removeFromSuperview];
                    [AdsClass adDismissed:[self class] forType:type];
                }

            }];
            [self.mngr set:view_ forType:type withListener:listener];
            [view_ setDelegate:[[AdsAdmobBannerListener alloc] init:[self.mngr getState:type] with:self]];
            [self.mngr setAutoKill:false forType:type];
            GADRequest *request = [GADRequest request];
            [view_ loadRequest:request];
        }
    }
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

-(BOOL)checkConsent:(BOOL) reset forUnderAge:(BOOL) underAge
{
  // Create a UMPRequestParameters object.
    UMPRequestParameters *parameters = [[UMPRequestParameters alloc] init];
    UMPDebugSettings *debugSettings=[[UMPDebugSettings alloc] init];
    debugSettings.testDeviceIdentifiers=@[ self.testID ];
    parameters.debugSettings=debugSettings;
  // Set tag for under age of consent. NO means users are not under age
  // of consent.
  parameters.tagForUnderAgeOfConsent = underAge;

	if (reset)
		[UMPConsentInformation.sharedInstance reset];
  // Request an update for the consent information.
  [UMPConsentInformation.sharedInstance
      requestConsentInfoUpdateWithParameters:parameters
      completionHandler:^(NSError *_Nullable requestConsentError) {
        if (requestConsentError) {
          [AdsClass adConsent:[self class] with:requestConsentError.localizedDescription andCode:(int)requestConsentError.code];
          return;
        }

      [UMPConsentForm loadWithCompletionHandler:^(UMPConsentForm * _Nullable consentForm, NSError * _Nullable error) {
          if (error) {
              [AdsClass adConsent:[self class] with:error.localizedDescription andCode:(int)error.code];
            return;
          }
          [consentForm presentFromViewController:[AdsClass getRootViewController] completionHandler:^(NSError * _Nullable error) {
              if (error) {
                  [AdsClass adConsent:[self class] with:error.localizedDescription andCode:(int)error.code];
                return;
              }
              [AdsClass adConsent:[self class] with:@"" andCode:0];
          }];
      }];
      /*
        [UMPConsentForm loadAndPresentIfRequiredFromViewController:[AdsClass getRootViewController]
            completionHandler:^(NSError *loadAndPresentError) {
              if (loadAndPresentError) {
		          [AdsClass adConsent:[self class] with:loadAndPresentError.localizedDescription andCode:loadAndPresentError.code];
                return;
              }

		      [AdsClass adConsent:[self class] with:@"" andCode:0];
            }];*/
      }]; 
	return TRUE;
}

-(UIView*)getView{
    return (UIView*)[self.mngr get:self.currentSize];
}

@end

@implementation AdsAdmobFullscreenListener

-(id)init:(AdsState*)state placeId:(NSString *)placeId ad:(NSObject *)ad with:(AdsAdmob*)instance{
    self.state = state;
    self.instance = instance;
    self.placeId=placeId;
    self.ad=ad;
    return self;
}

/// Tells the delegate that an impression has been recorded for the ad.
- (void)adDidRecordImpression:(nonnull id<GADFullScreenPresentingAd>)ad;
{
}

/// Tells the delegate that a click has been recorded for the ad.
- (void)adDidRecordClick:(nonnull id<GADFullScreenPresentingAd>)ad;
{
    [AdsClass adActionBegin:[self.instance class] forType:[self.state getType]];
}

/// Tells the delegate that the ad failed to present full screen content.
- (void)ad:(nonnull id<GADFullScreenPresentingAd>)ad
    didFailToPresentFullScreenContentWithError:(nonnull NSError *)error;
{
    [AdsClass adFailed:[self.instance class] with:[error localizedDescription] forType:[self.state getType]];
}

/// Tells the delegate that the ad presented full screen content.
- (void)adDidPresentFullScreenContent:(nonnull id<GADFullScreenPresentingAd>)ad
{
    [AdsClass adDisplayed:[self.instance class] forType:[self.state getType]];
}

/// Tells the delegate that the ad dismissed full screen content.
- (void)adDidDismissFullScreenContent:(nonnull id<GADFullScreenPresentingAd>)ad
{
    [AdsClass adDismissed:[self.instance class] forType:[self.state getType]];
    [self.instance loadAd:[NSMutableArray arrayWithObjects:[self.state getType],self.placeId, nil]];
}

@end

@implementation AdsAdmobBannerListener

-(id)init:(AdsState*)state with:(AdsAdmob*)instance{
    self.state = state;
    self.instance = instance;
    return self;
}

- (void)adViewDidReceiveAd:(GADBannerView *)bannerView{
    [AdsClass adReceived:[self.instance class] forType:[self.state getType]];
    [self.state load];
}

- (void)adView:(GADBannerView *)bannerView didFailToReceiveAdWithError:(NSError *)error{
    [AdsClass adFailed:[self.instance class] with:[error localizedDescription] forType:[self.state getType]];
}

- (void)adViewWillPresentScreen:(GADBannerView *)bannerView{
}

- (void)adViewWillDismissScreen:(GADBannerView *)bannerView{
}

@end
