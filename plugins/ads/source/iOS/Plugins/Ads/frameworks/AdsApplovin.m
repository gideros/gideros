//
//  AdsAdmob.m
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//
#include "gideros.h"
#import "AdsApplovin.h"
#import "AdsClass.h"

@implementation AdsApplovin
-(id)init{
    [ALSdk initializeSdk];
    self.appKey = @"";
    self.curType = @"";
    self.view_ = nil;
    self.mngr = [[AdsManager alloc] init];
    return self;
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
    ALSdk* sdk = [ALSdk sharedWithKey: self.appKey];
    if ([type isEqualToString:@"interstitial"]) {
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
        [listener setShow:^(){
            [AdsClass adDisplayed:[self class] forType:type];
            ALInterstitialAd* interstitial = [[ALInterstitialAd alloc] initInterstitialAdWithSdk: sdk];
            [interstitial showOver: [UIApplication sharedApplication].keyWindow andRender: (ALAd*)[self.mngr get:type]];
        }];
        [listener setDestroy:^(){
            [self hideAd:type];
        }];
        [listener setHide:^(){
        }];
        [self.mngr set:nil forType:type withListener:listener];
        
        ALAdService* adService = sdk.adService;
        [adService loadNextAd: [ALAdSize sizeInterstitial] andNotify: [[AdsApplovinListener alloc] init:[self.mngr getState:type] with:self]];
    }
    else
    {
            if([self.mngr get:type] == nil)
            {
                self.curType = [type copy];
                ALAdSize* banner = [ALAdSize sizeBanner];
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
                
                self.view_ = [[ALAdView alloc] initWithFrame: frame size: banner sdk: sdk];
                
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
                        self.view_.adLoadDelegate = nil;
                        self.view_.adDisplayDelegate = nil;
                        [self.view_ release];
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
                AdsApplovinListener *list =[[AdsApplovinListener alloc] init:[self.mngr getState:type] with:self];
                
                self.view_.adLoadDelegate = list;
                self.view_.adDisplayDelegate = list;
                [self.mngr setAutoKill:false forType:type];
                [self.view_ loadNextAd];

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
    return (UIView*)[self.mngr get:self.curType];
}

@end


@implementation AdsApplovinListener

-(id)init:(AdsState*)state with:(AdsApplovin*)instance{
    self.state = state;
    self.instance = instance;
    return self;
}

-(void) adService:(ALAdService *)adService didLoadAd:(ALAd *)ad
{
    if([[self.state getType] isEqual:@"interstitial"]){
        [self.state setObject:ad];
    }
    [AdsClass adReceived:[self.instance class] forType:[self.state getType]];
    [self.state load];
}

-(void) adService:(ALAdService *)adService didFailToLoadAdWithError:(int)code
{
    NSString *error = @"Unknown error";
    if(code == kALErrorCodeNoFill)
    {
        error = @"No fill";
    }
    else if(code == kALErrorCodeAdRequestNetworkTimeout){
        error = @"Network timeout";
    }
    else if(code == kALErrorCodeUnableToPrecacheResources){
        error = @"Unable to precache";
    }
    [AdsClass adFailed:[self.instance class] with:error forType:[self.state getType]];
    [self.state reset];
}

-(void) ad:(ALAd *)ad wasDisplayedIn:(UIView *)view{}

-(void) ad:(ALAd *)ad wasClickedIn:(UIView *)view
{
    [AdsClass adActionBegin:[self.instance class] forType:[self.state getType]];
}

-(void) ad:(ALAd *)ad wasHiddenIn:(UIView *)view{
    [AdsClass adDismissed:[self.instance class] forType:[self.state getType]];
}
@end

