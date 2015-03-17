//
//  AdsTapfortap.m
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//
#include "gideros.h"
#import "AdsTapfortap.h"
#import "AdsClass.h"

@implementation AdsTapfortap

-(id)init{
    self.mngr = [[AdsManager alloc] init];
    return self;
}

-(void)destroy{
    [self.mngr destroy];
    [self.mngr release];
    self.mngr = nil;
}

-(void)setKey:(NSMutableArray*)parameters{
    [TFTTapForTap initializeWithAPIKey: [parameters objectAtIndex:0]];
}

-(void)loadAd:(NSMutableArray*)parameters{
    NSString *type = [parameters objectAtIndex:0];
    if ([type isEqualToString:@"interstitial"]) {
        TFTInterstitial *interstitial_ = [TFTInterstitial interstitialWithDelegate:self];
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
        [listener setShow:^(){
            [AdsClass adDisplayed:[self class] forType:type];
            [interstitial_ showWithViewController:[AdsClass getRootViewController]];
        }];
        [listener setDestroy:^(){}];
        [listener setHide:^(){}];
        [self.mngr set:interstitial_ forType:type withListener:listener];
    }
    else if ([type isEqualToString:@"moreapps"]) {
        TFTAppWall *appWall_ = [TFTAppWall appWallWithDelegate:self];
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
        [listener setShow:^(){
            [AdsClass adDisplayed:[self class] forType:type];
            [appWall_ showWithViewController:[AdsClass getRootViewController]];
        }];
        [listener setDestroy:^(){
        }];
        [listener setHide:^(){}];
        [self.mngr set:appWall_ forType:type withListener:listener];
    }
    else
    {
            if([self.mngr get:type] == nil)
            {
                TFTBanner *view_ = [TFTBanner bannerWithFrame: CGRectMake(0, 0, 320, 50) delegate: self];
                AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
                [listener setShow:^(){
                    [AdsClass adDisplayed:[self class] forType:type];
                    UIViewController *c = [AdsClass getRootViewController];
                    [c.view addSubview:view_];
                }];
                [listener setDestroy:^(){
                    [self hideAd:type];
                    if(view_ != nil)
                    {
                        //view_.delegate = nil;
                        [view_ release];
                    }
                }];
                [listener setHide:^(){
                    if(view_ != nil)
                    {
                        if ([view_ superview] != nil)
                        {
                            [view_ removeFromSuperview];
                            [AdsClass adDismissed:[self class] forType:type];
                        }
                    }
                }];
                [self.mngr set:view_ forType:type withListener:listener];
                [self.mngr setAutoKill:false forType:type];
                [self.mngr load:type];
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
    [TFTTapForTap enableTestMode];
}

-(UIView*)getView{
    return (UIView*)[self.mngr get:@"banner"];
}

- (void)tftBannerDidReceiveAd:(TFTBanner *)banner{
    [AdsClass adReceived:[self class] forType:@"banner"];
    [self.mngr load:@"banner"];
}
- (void)tftBanner:(TFTBanner *)banner didFail:(NSString *)reason{
    [AdsClass adFailed:[self class] with:reason forType:@"banner"];
    [self.mngr reset:@"banner"];
}
- (void)tftBannerWasTapped:(TFTBanner *)banner{
    [AdsClass adActionBegin:[self class] forType:@"banner"];
}

- (void)tftInterstitialDidReceiveAd:(TFTInterstitial *)interstitial{
    [AdsClass adReceived:[self class] forType:@"interstitial"];
    [self.mngr load:@"interstitial"];
}
- (void)tftInterstitial:(TFTInterstitial *)interstitial didFail:(NSString *)reason{
    [AdsClass adFailed:[self class] with:reason forType:@"interstitial"];
    [self.mngr reset:@"interstitial"];
}
- (void)tftInterstitialDidShow:(TFTInterstitial *)interstitial{
}
- (void)tftInterstitialWasTapped:(TFTInterstitial *)interstitial{
    [AdsClass adActionBegin:[self class] forType:@"interstitial"];
}
- (void)tftInterstitialWasDismissed:(TFTInterstitial *)interstitial{
    [AdsClass adDismissed:[self class] forType:@"interstitial"];
}

- (void)tftAppWallDidReceiveAd:(TFTAppWall *)appWall{
    [AdsClass adReceived:[self class] forType:@"moreapps"];
    [self.mngr load:@"moreapps"];
}
- (void)tftAppWall:(TFTAppWall *)appWall didFail:(NSString *)reason{
    [AdsClass adFailed:[self class] with:reason forType:@"moreapps"];
    [self.mngr reset:@"moreapps"];
}
- (void)tftAppWallDidShow:(TFTAppWall *)appWall{

}
- (void)tftAppWallWasTapped:(TFTAppWall *)appWall{
    [AdsClass adActionBegin:[self class] forType:@"moreapps"];
}
- (void)tftAppWallWasDismissed:(TFTAppWall *)appWall{
    [AdsClass adDismissed:[self class] forType:@"moreapps"];
}

@end
