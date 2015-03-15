//
//  AdsMopub.m
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//
#include "gideros.h"
#import "AdsMopub.h"
#import "AdsClass.h"

@implementation AdsMopub

-(id)init{
    self.mngr = [[AdsManager alloc] init];
    self.appId = @"";
    return self;
}

-(void)destroy{
    [self.mngr destroy];
    [self.mngr release];
    self.mngr = nil;
}

-(void)setKey:(NSMutableArray*)parameters{
    self.appId = [parameters objectAtIndex:0];
}

-(void)loadAd:(NSMutableArray*)parameters{
    NSString *type = [parameters objectAtIndex:0];
    NSString *tag = nil;
    if([parameters count] > 1)
        tag = [parameters objectAtIndex:1];

    if ([type isEqualToString:@"interstitial"]) {
        MPInterstitialAdController *interstitial_;
        if(tag != nil)
            interstitial_ = [MPInterstitialAdController interstitialAdControllerForAdUnitId:tag];
        else
            interstitial_ = [MPInterstitialAdController interstitialAdControllerForAdUnitId:self.appId];
            
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
        [listener setShow:^(){
            [AdsClass adDisplayed:[self class] forType:type];
            [interstitial_ showFromViewController:[AdsClass getRootViewController]];
        }];
        [listener setDestroy:^(){}];
        [listener setHide:^(){}];
        [self.mngr set:interstitial_ forType:type withListener:listener];
        interstitial_.delegate = self;
        [interstitial_ loadAd];
    }
    else
    {
        MPAdView *view_;
        if(tag != nil)
            view_ = [[[MPAdView alloc] initWithAdUnitId:tag                                                          size:MOPUB_BANNER_SIZE] autorelease];
        else
            view_ = [[[MPAdView alloc] initWithAdUnitId:self.appId                                                          size:MOPUB_BANNER_SIZE] autorelease];
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
        [listener setShow:^(){
            self.currentSize = [type copy];
            [AdsClass adDisplayed:[self class] forType:type];
            [[AdsClass getRootViewController].view addSubview:view_];
        }];
        [listener setDestroy:^(){
            [self hideAd:type];
            if(view_ != nil)
            {
                [view_ release];
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
        [self.mngr setAutoKill:false forType:type];

        view_.delegate = self;
        [view_ loadAd];
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

- (UIViewController *)viewControllerForPresentingModalView{
    return [AdsClass getRootViewController];
}

- (void)adViewDidLoadAd:(MPAdView *)view{
    [AdsClass adReceived:[self class] forType:self.currentSize];
    [self.mngr load:@"banner"];
}

- (void)adViewDidFailToLoadAd:(MPAdView *)view{
    [AdsClass adFailed:[self class] with:@"Failed to load ad" forType:self.currentSize];
    [self.mngr reset:@"banner"];
}

- (void)willPresentModalViewForAd:(MPAdView *)view{
    [AdsClass adActionBegin:[self class] forType:self.currentSize];
}

- (void)didDismissModalViewForAd:(MPAdView *)view{
    [AdsClass adActionEnd:[self class] forType:self.currentSize];
}

- (void)willLeaveApplicationFromAd:(MPAdView *)view{
    [AdsClass adActionBegin:[self class] forType:self.currentSize];
}

- (void)interstitialDidLoadAd:(MPInterstitialAdController *)interstitial{
    [AdsClass adReceived:[self class] forType:@"interstitial"];
    [self.mngr load:@"interstitial"];
}

- (void)interstitialDidFailToLoadAd:(MPInterstitialAdController *)interstitial{
    [AdsClass adFailed:[self class] with:@"Failed to load ad" forType:@"interstitial"];
    [self.mngr reset:@"interstitial"];
}

- (void)interstitialDidDisappear:(MPInterstitialAdController *)interstitial{
    [AdsClass adDismissed:[self class] forType:@"interstitial"];
}

@end
