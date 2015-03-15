//
//  AdsHeyzap.m
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//
#include "gideros.h"
#import "AdsHeyzap.h"
#import "AdsClass.h"

@implementation AdsHeyzap

-(id)init{
    self.hasInterstitial = NO;
    self.hasVideo = NO;
    self.hasV4vc = NO;
    self.mngr = [[AdsManager alloc] init];
    return self;
}

-(void)destroy{
    [self.mngr destroy];
    [self.mngr release];
    self.mngr = nil;
}

-(void)setKey:(NSMutableArray*)parameters{
    [HZInterstitialAd setDelegate: self];
}

-(void)loadAd:(NSMutableArray*)parameters{
    if([parameters count] > 0)
    {
        NSString *type = [parameters objectAtIndex:0];
        NSString *tag = nil;
        if([parameters count] > 1)
            tag = [parameters objectAtIndex:1];
        if([type isEqualToString:@"video"])
        {
            AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
            [listener setShow:^(){
                [AdsClass adDisplayed:[self class] forType:type];
                if(tag != nil)
                    [HZVideoAd showForTag:tag];
                else
                    [HZVideoAd show];
            }];
            [listener setDestroy:^(){
                [self hideAd:type];
            }];
            [listener setHide:^(){
                [HZVideoAd hide];
            }];
            [self.mngr set:type forType:type withListener:listener];
            if(tag != nil)
                [HZVideoAd fetchForTag:tag];
            else
                [HZVideoAd fetch];
        }
        else if([type isEqualToString:@"v4vc"])
        {
            AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
            [listener setShow:^(){
                [AdsClass adDisplayed:[self class] forType:type];
                [HZIncentivizedAd show];
            }];
            [listener setDestroy:^(){
                [self hideAd:type];
            }];
            [listener setHide:^(){
                [HZIncentivizedAd hide];
            }];
            [self.mngr set:type forType:type withListener:listener];
            [HZIncentivizedAd fetch];
        }
        else{
            AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
            [listener setShow:^(){
                [AdsClass adDisplayed:[self class] forType:type];
                if(tag != nil)
                    [HZInterstitialAd showForTag:tag];
                else
                    [HZInterstitialAd show];
            }];
            [listener setDestroy:^(){
                [self hideAd:type];
            }];
            [listener setHide:^(){
                [HZInterstitialAd hide];
            }];
            [self.mngr set:type forType:type withListener:listener];
            if(tag != nil)
                [HZInterstitialAd fetchForTag:tag];
            else
                [HZInterstitialAd fetch];
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
    return nil;
}

-(void) didFailToShowAdWithTag:(NSString *)tag andError:(NSError *)error
{
    if([self.mngr get:@"interstitial"] != nil)
        [AdsClass adFailed:[self class] with:[error localizedDescription] forType:@"interstitial"];
    [self.mngr reset:@"interstitial"];
    if([self.mngr get:@"video"] != nil)
        [AdsClass adFailed:[self class] with:[error localizedDescription] forType:@"video"];
    [self.mngr reset:@"video"];
    if([self.mngr get:@"v4vc"] != nil)
        [AdsClass adFailed:[self class] with:[error localizedDescription] forType:@"v4vc"];
    [self.mngr reset:@"v4vc"];
}

- (void) didClickAdWithTag:(NSString *)tag{
    if(self.hasInterstitial)
        [AdsClass adActionBegin:[self class] forType:@"interstitial"];
    if(self.hasVideo)
        [AdsClass adActionBegin:[self class] forType:@"video"];
    if(self.hasV4vc)
        [AdsClass adActionBegin:[self class] forType:@"v4vc"];
}

- (void) didHideAdWithTag:(NSString *)tag
{
    if(self.hasInterstitial)
        [AdsClass adDismissed:[self class] forType:@"interstitial"];
    if(self.hasVideo)
        [AdsClass adDismissed:[self class] forType:@"video"];
    if(self.hasV4vc)
        [AdsClass adDismissed:[self class] forType:@"v4vc"];
}

- (void) didReceiveAdWithTag:(NSString *)tag
{
    if([HZInterstitialAd isAvailableForTag:tag])
    {
        [self.mngr load:@"interstitial"];
        [AdsClass adReceived:[self class] forType:@"interstitial"];
        self.hasInterstitial = YES;
    }
    if([HZVideoAd isAvailableForTag:tag])
    {
        [self.mngr load:@"video"];
        [AdsClass adReceived:[self class] forType:@"video"];
        self.hasVideo = YES;
    }
    
    if([HZIncentivizedAd isAvailable])
    {
        [self.mngr load:@"v4vc"];
        [AdsClass adReceived:[self class] forType:@"v4vc"];
        self.hasV4vc = YES;
    }
}

- (void) didFailToReceiveAdWithTag: (NSString *)tag{
    if([self.mngr get:@"interstitial"] != nil)
        [AdsClass adFailed:[self class] with:@"Failed to receive" forType:@"interstitial"];
    [self.mngr reset:@"interstitial"];
    if([self.mngr get:@"video"] != nil)
        [AdsClass adFailed:[self class] with:@"Failed to receive" forType:@"video"];
    [self.mngr reset:@"video"];
    if([self.mngr get:@"v4vc"] != nil)
        [AdsClass adFailed:[self class] with:@"Failed to receive" forType:@"v4vc"];
    [self.mngr reset:@"v4vc"];
}

- (void) didCompleteAd{
    self.hasV4vc = NO;
    [AdsClass adActionEnd:[self class] forType:@"v4vc"];
}

- (void) didFailToCompleteAd{
    self.hasV4vc = NO;
}

@end
