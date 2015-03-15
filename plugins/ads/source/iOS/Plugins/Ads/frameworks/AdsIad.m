//
//  AdsIad.m
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import "AdsIad.h"
#import "AdsClass.h"

@implementation AdsIad

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
    
}

-(void)loadAd:(NSMutableArray*)parameters{
    NSString *type = [parameters objectAtIndex:0];
    if ([type isEqualToString:@"interstitial"]) {
        if (floor(NSFoundationVersionNumber) > NSFoundationVersionNumber_iOS_6_1 || [[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
            ADInterstitialAd *interstitial_ = [[ADInterstitialAd alloc] init];
            interstitial_.delegate = self;
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
        }
        else{
            [AdsClass adFailed:[self class] with:@"Interstitials not supported" forType:type];
        }
    }
    else
    {
        if([self.mngr get:type] == nil)
        {
            self.curSize = [type copy];
            ADBannerView *view_ = [[ADBannerView alloc] initWithFrame:CGRectZero];
            if ([AdsClass isPortrait])
            {
                view_.currentContentSizeIdentifier = ADBannerContentSizeIdentifierPortrait;
            }
            else
            {
                view_.currentContentSizeIdentifier = ADBannerContentSizeIdentifierLandscape;
            }
            view_.delegate = self;
            AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
            [listener setShow:^(){
                [AdsClass adDisplayed:[self class] forType:type];
                [[AdsClass getRootViewController].view addSubview:view_];
            }];
            [listener setDestroy:^(){
                [self hideAd:type];
                if(view_ != nil)
                {
                    view_.delegate = nil;
                    [view_ cancelBannerViewAction];
                    [view_ release];
                }
            }];
            [listener setHide:^(){
                if(view_ != nil)
                {
                    if (view_.superview != nil)
                    {
                        [view_ removeFromSuperview];
                        [AdsClass adDismissed:[self class] forType:type];
                    }
                }
            }];
            [self.mngr set:view_ forType:type withListener:listener];
            [self.mngr setAutoKill:false forType:type];
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
    return (UIView*)[self.mngr get:self.curSize];
}

- (void)bannerViewDidLoadAd:(ADBannerView *)banner
{
    [self.mngr load:self.curSize];
    [AdsClass adReceived:[self class] forType:self.curSize];
}

- (BOOL)bannerViewActionShouldBegin:(ADBannerView *)banner willLeaveApplication:(BOOL)willLeave
{
    [AdsClass adActionBegin:[self class] forType:self.curSize];
	return YES;
}

- (void)bannerViewActionDidFinish:(ADBannerView *)banner
{
    [AdsClass adActionEnd:[self class] forType:self.curSize];
}

- (void)bannerView:(ADBannerView *)banner didFailToReceiveAdWithError:(NSError *)error
{
    [self.mngr reset:self.curSize];
    [AdsClass adFailed:[self class] with:[error localizedDescription] forType:self.curSize];
}

- (void)interstitialAdDidUnload:(ADInterstitialAd *)interstitialAd
{
    [AdsClass adDismissed:[self class] forType:@"interstitial"];
}

- (void)interstitialAd:(ADInterstitialAd *)interstitialAd didFailWithError:(NSError *)error
{
    [AdsClass adFailed:[self class] with:[error localizedDescription] forType:@"interstitial"];
    [self.mngr reset:@"interstitial"];
}

- (void)interstitialAdDidLoad:(ADInterstitialAd *)interstitialAd
{
    [AdsClass adReceived:[self class] forType:@"interstitial"];
    [self.mngr load:@"interstitial"];
}

- (BOOL)interstitialAdActionShouldBegin:(ADInterstitialAd *)interstitialAd willLeaveApplication:(BOOL)willLeave
{
    [AdsClass adActionBegin:[self class] forType:@"interstitial"];
    return YES;
}

- (void)interstitialAdActionDidFinish:(ADInterstitialAd *)interstitialAd
{
    [AdsClass adActionEnd:[self class] forType:@"interstitial"];
}
@end
