//
//  AdsAdmob.m
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//
#include "gideros.h"
#import "AdsAmazon.h"
#import "AdsClass.h"

@implementation AdsAmazon
-(id)init{
    self.currentType = AmazonAdSize_320x50;
    self.currentSize = @"banner";
    self.appKey = @"";
    self.test = NO;
    self.mngr = [[AdsManager alloc] init];
    return self;
}

-(CGSize)getAdType:(NSString*)type{
    if([type isEqual:@"320x50"])
    {
        return AmazonAdSize_320x50;
    }
    else if([type isEqual:@"300x50"])
    {
        return AmazonAdSize_300x50;
    }
    else if([type isEqual:@"300x250"])
    {
        return AmazonAdSize_300x250;
    }
    else if([type isEqual:@"728x90"])
    {
        return AmazonAdSize_728x90;
    }
    else if([type isEqual:@"1024x50"])
    {
        return AmazonAdSize_1024x50;
    }
    else if([type isEqual:@"auto"])
    {
        return [self getAutoSize];
    }
    return AmazonAdSize_320x50;
}

- (CGSize)getAutoSize{
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
    
    int maparray[3][3] = { { 1, 1024, 50 },
        { 1, 728, 90 },
        { 3, 320, 50 } };
    int result = 3;
    for (int i = 0; i < 3; i++) {
        if (maparray[i][1] <= screenWidth
            && maparray[i][2] <= screenHeight) {
            result = maparray[i][0];
            if(result == 1){ return AmazonAdSize_1024x50;}
            else if(result == 2){ return AmazonAdSize_728x90;}
            else
                return AmazonAdSize_320x50;
        }
    }
    return AmazonAdSize_320x50;
    
}


-(void)destroy{
    [self.mngr destroy];
    [self.mngr release];
    self.mngr = nil;
}

-(void)setKey:(NSMutableArray*)parameters{
    [[AmazonAdRegistration sharedRegistration] setAppKey:[parameters objectAtIndex:0]];
}

-(void)loadAd:(NSMutableArray*)parameters{
    NSString *type = [parameters objectAtIndex:0];
    if ([type isEqualToString:@"interstitial"]) {
        AmazonAdInterstitial *interstitial_ = [AmazonAdInterstitial amazonAdInterstitial];
        
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
        [listener setShow:^(){
            [AdsClass adDisplayed:[self class] forType:type];
            [interstitial_ presentFromViewController:[AdsClass getRootViewController]];
        }];
        [listener setDestroy:^(){
            [self hideAd:type];
        }];
        [listener setHide:^(){
            [interstitial_ release];
        }];
        [self.mngr set:interstitial_ forType:type withListener:listener];

        interstitial_.delegate = [[AdsAmazonListener alloc] init:[self.mngr getState:type] with:self];
        // Set the adOptions.
        AmazonAdOptions *options = [AmazonAdOptions options];
        if(!self.test)
        {
            // Turn on isTestRequest to load a test ad
            options.isTestRequest = YES;
        }
        [interstitial_ load:options];
        
    }
    else
    {
            if([self.mngr get:type] == nil)
            {
                self.currentType = [self getAdType:type];
                AmazonAdView *view_ = [AmazonAdView amazonAdViewWithAdSize:self.currentType];
                
                AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
                [listener setShow:^(){
                    self.currentSize = [type copy];
                    [AdsClass adDisplayed:[self class] forType:type];
                    UIViewController *root = [AdsClass getRootViewController];
                    [root.view addSubview:view_];
                }];
                [listener setDestroy:^(){
                    [self hideAd:type];
                }];
                [listener setHide:^(){
                    if(view_ != nil && [view_ superview] != nil)
                    {
                        [view_ removeFromSuperview];
                        [AdsClass adDismissed:[self class] forType:type];
                    }

                }];
                [self.mngr set:view_ forType:type withListener:listener];
                [self.mngr setAutoKill:false forType:type];
                
                // Set the adOptions.
                AmazonAdOptions *options = [AmazonAdOptions options];
                view_.delegate = [[AdsAmazonListener alloc] init:[self.mngr getState:type] with:self];

                if(!self.test)
                {
                    // Turn on isTestRequest to load a test ad
                    options.isTestRequest = YES;
                }
                // Call loadAd
                [view_ loadAd:options];

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
    self.test = YES;
}

-(UIView*)getView{
    return (UIView*)[self.mngr get:self.currentSize];
}
@end


@implementation AdsAmazonListener

-(id)init:(AdsState*)state with:(AdsAmazon*)instance{
    self.state = state;
    self.instance = instance;
    return self;
}

- (UIViewController *)viewControllerForPresentingModalView {
    return [AdsClass getRootViewController];
}

- (void)adViewWillExpand:(AmazonAdView *)view {
    [AdsClass adActionBegin:[self.instance class] forType:[self.state getType]];
}

- (void)adViewDidCollapse:(AmazonAdView *)view {
    [AdsClass adActionEnd:[self.instance class] forType:[self.state getType]];
}

- (void)adViewDidLoad:(AmazonAdView *)view {
    [AdsClass adReceived:[self.instance class] forType:[self.state getType]];
    [self.state load];
}

- (void)adViewDidFailToLoad:(AmazonAdView *)view withError:(AmazonAdError *)error {
    [AdsClass adFailed:[self.instance class] with:[error errorDescription] forType:[self.state getType]];
    [self.state reset];
}

- (void)interstitialDidLoad:(AmazonAdInterstitial *)interstitial
{
    [AdsClass adReceived:[self.instance class] forType:[self.state getType]];
    [self.state load];

}

- (void)interstitialDidFailToLoad:(AmazonAdInterstitial *)interstitial withError:(AmazonAdError *)error
{
    [AdsClass adFailed:[self.instance class] with:[error errorDescription] forType:[self.state getType]];
    [self.state reset];
}

- (void)interstitialWillPresent:(AmazonAdInterstitial *)interstitial
{

}

- (void)interstitialDidPresent:(AmazonAdInterstitial *)interstitial
{

}

- (void)interstitialWillDismiss:(AmazonAdInterstitial *)interstitial
{
}

- (void)interstitialDidDismiss:(AmazonAdInterstitial *)interstitial
{

}

@end
