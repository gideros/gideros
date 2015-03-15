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

@implementation AdsAdmob
-(id)init{
    self.testID = @"";
    self.currentType = &kGADAdSizeBanner;
    self.currentSize = @"banner";
    self.appKey = @"";
    self.mngr = [[AdsManager alloc] init];
    return self;
}

-(const GADAdSize*)getAdType:(NSString*)type{
    if([type isEqual:@"banner"])
    {
        return &kGADAdSizeBanner;
    }
    else if([type isEqual:@"iab_mrect"])
    {
        return &kGADAdSizeMediumRectangle;
    }
    else if([type isEqual:@"iab_banner"])
    {
        return &kGADAdSizeFullBanner;
    }
    else if([type isEqual:@"iab_leaderboard"])
    {
        return &kGADAdSizeLeaderboard;
    }
    else if([type isEqual:@"smart_banner"] || [type isEqual:@"auto"])
    {
        if ([AdsClass isPortrait])
        {
            return &kGADAdSizeSmartBannerPortrait;
        }
        else
        {
            return &kGADAdSizeSmartBannerLandscape;
        }
    }
    return &kGADAdSizeBanner;
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
        GADInterstitial *interstitial_ = [[GADInterstitial alloc] init];
        
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
        [listener setShow:^(){
            [AdsClass adDisplayed:[self class] forType:type];
            [interstitial_ presentFromRootViewController:[AdsClass getRootViewController]];
        }];
        [listener setDestroy:^(){
            [self hideAd:type];
        }];
        [listener setHide:^(){
            [interstitial_ release];
        }];
        [self.mngr set:interstitial_ forType:type withListener:listener];

        [interstitial_ setDelegate:self];
        interstitial_.adUnitID = placeId;
        GADRequest *request = [GADRequest request];
        if(![self.testID isEqualToString:@""])
        {
            request.testDevices = @[self.testID];
        }
        [interstitial_ loadRequest:request];
    }
    else
    {
            if([self.mngr get:type] == nil)
            {
                self.currentType = [self getAdType:type];
                GADBannerView *view_ = [[GADBannerView alloc] initWithAdSize:*self.currentType];
                view_.adUnitID = placeId;
                view_.rootViewController = [AdsClass getRootViewController];
                
                AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
                [listener setShow:^(){
                    self.currentSize = [type copy];
                    [AdsClass adDisplayed:[self class] forType:type];
                    [view_.rootViewController.view addSubview:view_];
                }];
                [listener setDestroy:^(){
                    [self hideAd:type];
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
                [view_ setDelegate:[[AdsAdmobListener alloc] init:[self.mngr getState:type] with:self]];
                [self.mngr setAutoKill:false forType:type];
                GADRequest *request = [GADRequest request];
                if(![self.testID isEqualToString:@""])
                {
                    request.testDevices = @[self.testID];
                }
                [view_ loadRequest:request];

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
    NSString *test;
    if (NSClassFromString(@"ASIdentifierManager")) {
        test = [[[ASIdentifierManager sharedManager]
                advertisingIdentifier] UUIDString];
        // Create pointer to the string as UTF8
        const char *ptr = [test UTF8String];
        
        // Create byte array of unsigned chars
        unsigned char md5Buffer[CC_MD5_DIGEST_LENGTH];
        
        // Create 16 byte MD5 hash value, store in buffer
        CC_MD5(ptr, strlen(ptr), md5Buffer);
        
        // Convert MD5 value in the buffer to NSString of hex values
        NSMutableString *output = [NSMutableString stringWithCapacity:CC_MD5_DIGEST_LENGTH * 2];
        for(int i = 0; i < CC_MD5_DIGEST_LENGTH; i++)
            [output appendFormat:@"%02x",md5Buffer[i]];
        self.testID = output;
    }
}

-(UIView*)getView{
    return (UIView*)[self.mngr get:self.currentSize];
}

-(void)interstitialDidReceiveAd:(GADInterstitial *)ad{
    [AdsClass adReceived:[self class] forType:@"interstitial"];
    [self.mngr load:@"interstitial"];
}

-(void)interstitial:(GADInterstitial *)ad didFailToReceiveAdWithError:(GADRequestError *)error{
    [AdsClass adFailed:[self class] with:[error localizedDescription] forType:@"interstitial"];
    [self.mngr reset:@"interstitial"];
}

-(void)interstitialWillPresentScreen:(GADInterstitial *)ad{
    [AdsClass adActionBegin:[self class] forType:@"interstitial"];
}

-(void)interstitialWillDismissScreen:(GADInterstitial *)ad{
    [AdsClass adDismissed:[self class] forType:@"interstitial"];
}

@end


@implementation AdsAdmobListener

-(id)init:(AdsState*)state with:(AdsAdmob*)instance{
    self.state = state;
    self.instance = instance;
    return self;
}

- (void)adViewDidReceiveAd:(GADBannerView *)bannerView{
    [AdsClass adReceived:[self.instance class] forType:[self.state getType]];
    [self.state load];
}

- (void)adView:(GADBannerView *)bannerView didFailToReceiveAdWithError:(GADRequestError *)error{
    [AdsClass adFailed:[self.instance class] with:[error localizedDescription] forType:[self.state getType]];
    [self.state reset];
}

- (void)adViewWillPresentScreen:(GADBannerView *)bannerView{
    [AdsClass adActionBegin:[self.instance class] forType:[self.state getType]];
}

- (void)adViewWillDismissScreen:(GADBannerView *)bannerView{
    [AdsClass adActionEnd:[self.instance class] forType:[self.state getType]];
}

@end
