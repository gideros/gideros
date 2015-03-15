//
//  AdsRevmob.m
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//
#include "gideros.h"
#import "AdsRevmob.h"
#import "AdsClass.h"

@implementation AdsRevmob

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
    [RevMobAds startSessionWithAppID:[parameters objectAtIndex:0]];
}

-(void)loadAd:(NSMutableArray*)parameters{
    NSString *type = [parameters objectAtIndex:0];
    NSString *tag = nil;
    if([parameters count] > 1)
        tag = [parameters objectAtIndex:1];

    if ([type isEqualToString:@"interstitial"]) {
        RevMobFullscreen *ad;
        if(tag != nil)
            ad = [[RevMobAds session]fullscreenWithPlacementId:tag];
        else
            ad = [[RevMobAds session] fullscreen];
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
        [listener setShow:^(){
            [AdsClass adDisplayed:[self class] forType:type];
            [ad showAd];
        }];
        [listener setDestroy:^(){}];
        [listener setHide:^(){}];
        [self.mngr set:ad forType:type withListener:listener];
        ad.delegate = [[AdsRevmobListener alloc] init:[self.mngr getState:type] with:self];
        [ad loadAd];
    }
    else if ([type isEqualToString:@"link"]) {
        RevMobAdLink *ad;
        if(tag != nil)
            ad = [[RevMobAds session] adLinkWithPlacementId:tag];
        else
            ad = [[RevMobAds session] adLink];
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
        [listener setShow:^(){
            [AdsClass adDisplayed:[self class] forType:type];
            [ad openLink];
        }];
        [listener setDestroy:^(){}];
        [listener setHide:^(){}];
        [self.mngr set:ad forType:type withListener:listener];
        ad.delegate = [[AdsRevmobListener alloc] init:[self.mngr getState:type] with:self];
    }
    else if ([type isEqualToString:@"popup"]) {
        RevMobPopup *ad;
        if(tag != nil)
            ad = [[RevMobAds session] popupWithPlacementId:tag];
        else
            ad = [[RevMobAds session] popup];
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
        [listener setShow:^(){
            [AdsClass adDisplayed:[self class] forType:type];
            [ad showAd];
        }];
        [listener setDestroy:^(){}];
        [listener setHide:^(){}];
        [self.mngr set:ad forType:type withListener:listener];
        ad.delegate = [[AdsRevmobListener alloc] init:[self.mngr getState:type] with:self];
    }
    else
    {
        RevMobBannerView *view_;
        if(tag != nil)
            view_ = [[RevMobAds session] bannerViewWithPlacementId:tag];
        else
            view_ = [[RevMobAds session] bannerView];
        if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) {
            [view_ setFrame:CGRectMake(0, 0, 768, 114)];
        } else {
            [view_ setFrame:CGRectMake(0, 0, 320, 50)];
        }
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
        [listener setShow:^(){
            [AdsClass adDisplayed:[self class] forType:type];
            self.currentSize = [type copy];
            [[AdsClass getRootViewController].view addSubview:view_];
        }];
        [listener setDestroy:^(){
            [self hideAd:type];
        }];
        [listener setHide:^(){
            if(view_ != nil)
            {
                [[RevMobAds session] hideBanner];
                [view_ removeFromSuperview];
                [AdsClass adDismissed:[self class] forType:type];
            }
        }];
        [self.mngr set:view_ forType:type withListener:listener];
        [self.mngr setAutoKill:false forType:type];
        view_.delegate = [[AdsRevmobListener alloc] init:[self.mngr getState:type] with:self];
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
    [RevMobAds session].testingMode = RevMobAdsTestingModeWithAds;
}

-(UIView*)getView{
    return (UIView*)[self.mngr get:self.currentSize];
}

@end


@implementation AdsRevmobListener

-(id)init:(AdsState*)state with:(AdsRevmob*)instance{
    self.state = state;
    self.instance = instance;
    return self;
}

- (void)revmobAdDidFailWithError:(NSError *)error {
    [AdsClass adFailed:[self.instance class] with:[error localizedDescription] forType:[self.state getType]];
    [self.state reset];
}

- (void)revmobAdDidReceive {
    [AdsClass adReceived:[self.instance class] forType:[self.state getType]];
    [self.state load];
}

- (void)revmobAdDisplayed {}

- (void)revmobUserClickedInTheAd {
    [AdsClass adActionBegin:[self.instance class] forType:[self.state getType]];
}

- (void)revmobUserClosedTheAd {
    [AdsClass adActionEnd:[self.instance class] forType:[self.state getType]];
}
@end
