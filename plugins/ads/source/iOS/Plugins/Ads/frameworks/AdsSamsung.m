//
//  AdsSamsung.m
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//
#include "gideros.h"
#import "AdsSamsung.h"
#import "AdsClass.h"

@implementation AdsSamsung

-(id)init{
    self.currentSize = @"small_banner";
    self.appKey = @"";
    self.mngr = [[AdsManager alloc] init];
    return self;
}

-(const AdHubAdSize*)getAdType:(NSString*)type{
    if([type isEqual:@"small_banner"])
    {
        return &kAdHubAdSize_B_320x50;
    }
    else if([type isEqual:@"medium_banner"])
    {
        return &kAdHubAdSize_B_640x100;
    }
    else if([type isEqual:@"big_banner"])
    {
        return &kAdHubAdSize_B_728x90;
    }
    else if([type isEqual:@"rectangle_banner"])
    {
        return &kAdHubAdSize_B_300x250;
    }
    else
    {
        return [self getAutoSize];
    }
}

-(void)destroy{
    [self.mngr destroy];
    [self.mngr release];
    self.mngr = nil;
}

-(void)setKey:(NSMutableArray*)parameters{
    self.appKey = [parameters objectAtIndex:0];
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
        AdHubInterstitial *interstitial = [[AdHubInterstitial alloc]
                                           initWithInventoryID:placeId];
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
        [listener setShow:^(){
            [interstitial presentFromViewController:[AdsClass getRootViewController]];
        }];
        [listener setDestroy:^(){}];
        [listener setHide:^(){}];
        [self.mngr set:interstitial forType:type withListener:listener];
        interstitial.delegate = self;
        [self.mngr load:type];
    }
    else if ([type isEqualToString:@"video"]) {
        AdHubPlayer *adPlayer = [[AdHubPlayer alloc]initWithInventoryID:self.appKey];
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
        [listener setShow:^(){
            [adPlayer startAdAppLaunchRoll];
        }];
        [listener setDestroy:^(){
            [self hideAd:type];
        }];
        [listener setHide:^(){
            //[adPlayer release];
        }];
        [self.mngr set:adPlayer forType:type withListener:listener];
        [self.mngr load:type];
        adPlayer.delegate = self;
    }
    else
    {
        if(![placeId isEqual:@""])
        {
            if([self.mngr get:type] == nil && [type isEqualToString:@"rectangle_banner"])
            {
                AdHubBoxBannerView *view2_ = [[AdHubBoxBannerView alloc]
                               initWithAdSize:*[self getAdType:type]
                               origin:CGPointMake(0.0, 0.0)
                               inventoryID:placeId];
                
                AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
                [listener setShow:^(){
                    self.currentSize = [type copy];
                    [AdsClass adDisplayed:[self class] forType:type];
                    [view2_.rootViewController.view addSubview:view2_];
                }];
                [listener setDestroy:^(){
                    [self hideAd:type];
                    if(view2_ != nil)
                    {
                        [view2_ release];
                    }
                }];
                [listener setHide:^(){
                    if(view2_ != nil)
                    {
                        [view2_ removeFromSuperview];
                        [AdsClass adDismissed:[self class] forType:type];
                    }
                }];
                [self.mngr set:view2_ forType:type withListener:listener];
                [self.mngr setAutoKill:false forType:type];
                [view2_ setDelegate:[[AdsSamsungListener alloc] init:[self.mngr getState:type] with:self]];
                if([AdsClass isPortrait])
                    [view2_ setPortrait];
                else
                    [view2_ setLandscape];
                view2_.rootViewController = [AdsClass getRootViewController];
                [view2_ startAd];
            }
            else if([self.mngr get:type] == nil)
            {
                AdHubBarBannerView *view1_ = [[AdHubBarBannerView alloc]
                              initWithAdSize:*[self getAdType:type]
                              yPositionPortrait:0
                              yPositionLandscape:0
                              inventoryID:placeId];
                AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
                [listener setShow:^(){
                    self.currentSize = [type copy];
                    [AdsClass adDisplayed:[self class] forType:type];
                    [view1_.rootViewController.view addSubview:view1_];
                }];
                [listener setDestroy:^(){
                    [self hideAd:type];
                    if(view1_ != nil)
                    {
                        [view1_ release];
                    }
                }];
                [listener setHide:^(){
                    if(view1_ != nil)
                    {
                        [view1_ removeFromSuperview];
                        [AdsClass adDismissed:[self class] forType:type];
                    }
                }];
                [self.mngr set:view1_ forType:type withListener:listener];
                [self.mngr setAutoKill:false forType:type];

                
                [view1_ setDelegate:[[AdsSamsungListener alloc] init:[self.mngr getState:type] with:self]];
                if([AdsClass isPortrait])
                    [view1_ setPortrait];
                else
                    [view1_ setLandscape];
     
                view1_.rootViewController = [AdsClass getRootViewController];
                [view1_ startAd];
            }
        }
        else
        {
            [AdsClass adError:[self class] with:[NSString stringWithFormat:@"Unknown type: %@", type]];
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

-(UIView*)getView{
    return (UIView*)[self.mngr get:self.currentSize];
}

- (const AdHubAdSize*)getAutoSize{
    float screenWidth = 0;
    float screenHeight = 0;
    CGRect screenRect = [[UIScreen mainScreen] bounds];
    if ([AdsClass isPortrait])
    {
        screenWidth = screenRect.size.width;
        screenHeight = screenRect.size.height;
    }
    else
    {
        screenWidth = screenRect.size.height;
        screenHeight = screenRect.size.width;
    }
    
    int maparray[3][3] = { { 1, 728, 90 }, {
        2, 640, 100 }, {
            3, 320, 50 } };
    int result = 3;
    for (int i = 0; i < 3; i++) {
        if (maparray[i][1] <= screenWidth
            && maparray[i][2] <= screenHeight) {
            result = maparray[i][0];
            if(result == 1){ return &kAdHubAdSize_B_728x90;}
            else if(result == 2){ return &kAdHubAdSize_B_640x100;}
            else
                return &kAdHubAdSize_B_320x50;
        }
    }
    return &kAdHubAdSize_B_320x50;
    
}

-(void)interstitialAdDidUnload:( AdHubInterstitial *)interstitialAd {
    [AdsClass adDismissed:[self class] forType:@"interstitial"];
}
-(void)interstitialAd:( AdHubInterstitial *)interstitialAd
     didFailWithError:(AdHubRequestError *)error {
    [AdsClass adFailed:[self class] with:@"Failed to load ad" forType:@"interstitial"];
    [self.mngr reset:@"interstitial"];
}
-(void)interstitialAdWillLoad:( AdHubInterstitial *)interstitialAd {
   [AdsClass adReceived:[self class] forType:@"interstitial"];
}
-(void)interstitialAdDidLoad:( AdHubInterstitial *)interstitialAd {

}

-(void)adHubPlayerClose{
    [AdsClass adDismissed:[self class] forType:@"video"];
}
-(void)adHubPlayerError {
    [AdsClass adFailed:[self class] with:@"Could not play the video" forType:@"video"];
    [self.mngr reset:@"video"];
}

@end


@implementation AdsSamsungListener

-(id)init:(AdsState*)state with:(AdsSamsung*)instance{
    self.state = state;
    self.instance = instance;
    return self;
}

- (void) bannerViewDidLoadAd: (AdHubView*)banner {
    [AdsClass adReceived:[self.instance class] forType:[self.state getType]];
    [self.state load];
}

-(void) bannerView: (AdHubView*)banner didFailToReceiveAdWithError:
(AdHubRequestError*)error {
    [AdsClass adFailed:[self.instance class] with:@"Failed to load ad" forType:[self.state getType]];
    [self.state reset];
}

@end
