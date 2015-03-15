//
//  AdsInmobi.m
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//
#include "gideros.h"
#import "AdsInmobi.h"
#import "AdsClass.h"

const static int types[6] = {IM_UNIT_320x48, IM_UNIT_300x250, IM_UNIT_728x90, IM_UNIT_468x60, IM_UNIT_120x600, IM_UNIT_320x50};


static CGRect rects[6];

@implementation AdsInmobi

-(id)init{
    self.currentType = IM_UNIT_320x50;
    self.mngr = [[AdsManager alloc] init];
    self.sizes = [NSMutableDictionary dictionary];
    [self.sizes setObject:@0 forKey:@"320x48"];
    [self.sizes setObject:@1 forKey:@"300x250"];
    [self.sizes setObject:@2 forKey:@"728x90"];
    [self.sizes setObject:@3 forKey:@"468x60"];
    [self.sizes setObject:@4 forKey:@"120x600"];
    [self.sizes setObject:@5 forKey:@"320x50"];
    [self.sizes setObject:@6 forKey:@"auto"];
    rects[0] = CGRectMake(0, 0, 320, 48);
    rects[1] = CGRectMake(0, 0, 300, 250);
    rects[2] = CGRectMake(0, 0, 728, 90);
    rects[3] = CGRectMake(0, 0, 468, 60);
    rects[4] = CGRectMake(0, 0, 120, 600);
    rects[5] = CGRectMake(0, 0, 320, 50);
    self.appKey = @"";
    
    return self;
}

-(void)destroy{
    [self.mngr destroy];
    [self.mngr release];
    self.mngr = nil;
}

-(void)setKey:(NSMutableArray*)parameters{
    self.appKey = [parameters objectAtIndex:0];
    [InMobi initialize:self.appKey];
}

-(void)loadAd:(NSMutableArray*)parameters{
    NSString *type = [parameters objectAtIndex:0];
    if ([type isEqualToString:@"interstitial"]) {
        IMInterstitial *interstitial_ = [[IMInterstitial alloc] initWithAppId:self.appKey];
        
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
        [listener setShow:^(){
            [AdsClass adDisplayed:[self class] forType:type];
            [interstitial_ presentInterstitialAnimated:YES];
        }];
        [listener setDestroy:^(){
            if(interstitial_ != nil)
            {
                interstitial_.delegate = nil;
            }
        }];
        [listener setHide:^(){}];
        [self.mngr set:interstitial_ forType:type withListener:listener];

        interstitial_.delegate = self;
        [interstitial_ loadInterstitial];

    }
    else
    {
        if([self.sizes objectForKey:type] != nil)
        {
            if([self.mngr get:type] == nil)
            {
                int t = [[self.sizes objectForKey:type] intValue];
                if(t == 6)
                {
                    t = [self getAutoSize];
                }
                if(self.currentType != types[t])
                {
                    self.currentType = types[t];
                }

                IMBanner  *view_ = [[IMBanner alloc] initWithFrame:rects[t] appId:self.appKey adSize:self.currentType];
                AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
                [listener setShow:^(){
                    self.currentSize = [type copy];
                    [AdsClass adDisplayed:[self class] forType:type];
                    [[AdsClass getRootViewController].view addSubview:view_];

                }];
                [listener setDestroy:^(){
                    [self hideAd: type];
                    if(view_ != nil)
                    {
                        view_.delegate = nil;
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
                view_.delegate = [[AdsInmobiListener alloc] init:[self.mngr getState:type] with:self];
                [view_ loadBanner];
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
    [InMobi setLogLevel:IMLogLevelDebug];
}

-(UIView*)getView{
    return (UIView*)[self.mngr get:self.currentSize];
}

- (void)interstitialDidReceiveAd:(IMInterstitial *)ad{
    [self.mngr load:@"interstitial"];
    [AdsClass adReceived:[self class] forType:@"interstitial"];
}

- (void)interstitial:(IMInterstitial *)ad
didFailToReceiveAdWithError:(IMError *)error{
    [AdsClass adFailed:[self class] with:[error localizedDescription] forType:@"interstitial"];
    [self.mngr reset:@"interstitials"];
}

- (void)interstitialWillPresentScreen:(IMInterstitial *)ad{

}

- (void)interstitial:(IMInterstitial *)ad didFailToPresentScreenWithError:(IMError *)error{
    [AdsClass adFailed:[self class] with:[error localizedDescription] forType:@"interstitial"];
}

- (void)interstitialWillDismissScreen:(IMInterstitial *)ad{
    
}

- (void)interstitialDidDismissScreen:(IMInterstitial *)ad{
    [AdsClass adDismissed:[self class] forType:@"interstitial"];
}

- (void)interstitialWillLeaveApplication:(IMInterstitial *)ad{
    [AdsClass adActionEnd:[self class] forType:@"interstitial"];
}

- (int)getAutoSize{
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
    
    int maparray[3][3] = { { 2, 728, 90 }, {
        3, 468, 60 }, {
            5, 320, 50 } };
    
    for (int i = 0; i < 3; i++) {
        if (maparray[i][1] <= screenWidth
            && maparray[i][2] <= screenHeight) {
            return maparray[i][0];
        }
    }
    return 5;
}


@end


@implementation AdsInmobiListener

-(id)init:(AdsState*)state with:(AdsInmobi*)instance{
    self.state = state;
    self.instance = instance;
    return self;
}

- (void)bannerDidReceiveAd:(IMBanner *)banner{
    [self.state load];
    [AdsClass adReceived:[self.instance class] forType:[self.state getType]];
}

- (void)banner:(IMBanner *)banner didFailToReceiveAdWithError:(IMError *)error{
    [self.state reset];
    [AdsClass adFailed:[self.instance class] with:[error localizedDescription] forType:[self.state getType]];
}

- (void)bannerWillPresentScreen:(IMBanner *)banner{
    
}

- (void)bannerWillDismissScreen:(IMBanner *)banner{
    
}

- (void)bannerDidDismissScreen:(IMBanner *)banner{
    [AdsClass adActionEnd:[self.instance class] forType:[self.state getType]];
}

- (void)bannerWillLeaveApplication:(IMBanner *)banner{
    [AdsClass adActionBegin:[self.instance class] forType:[self.state getType]];
}

@end
