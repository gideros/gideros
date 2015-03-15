//
//  AdsChartboost.m
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//
#include "gideros.h"
#import "AdsChartboost.h"
#import "AdsClass.h"
#import <Chartboost/CBNewsfeedUI.h>

@implementation AdsChartboost
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
    [Chartboost startWithAppId:[parameters objectAtIndex:0] appSignature:[parameters objectAtIndex:1] delegate:self];
    [CBNewsfeed startWithDelegate:self];
}

-(void)loadAd:(NSMutableArray*)parameters{
    NSString *type = [parameters objectAtIndex:0];
    NSString *tag = CBLocationDefault;
    if([parameters count] > 1)
        tag = [parameters objectAtIndex:1];
    if ([type isEqualToString:@"interstitial"] || [type isEqualToString:@"auto"]) {
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
        [listener setShow:^(){
            [AdsClass adDisplayed:[self class] forType:type];
            [Chartboost showInterstitial:tag];
        }];
        [listener setDestroy:^(){}];
        [listener setHide:^(){}];
        [self.mngr set:listener forType:type withListener:listener];
        if([Chartboost hasInterstitial:tag]){
            [self.mngr load:type];
            [AdsClass adReceived:[self class] forType:type];
        }
        else
            [Chartboost cacheInterstitial:tag];
    }
    else if ([type isEqualToString:@"moreapps"]) {
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
        [listener setShow:^(){
            [AdsClass adDisplayed:[self class] forType:type];
            [Chartboost showMoreApps:tag];
        }];
        [listener setDestroy:^(){}];
        [listener setHide:^(){}];
        [self.mngr set:listener forType:type withListener:listener];
        if([Chartboost hasMoreApps:tag]){
            [self.mngr load:type];
            [AdsClass adReceived:[self class] forType:type];
        }
        else
            [Chartboost cacheMoreApps:tag];
    }
    else if ([type isEqualToString:@"feed"]) {
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
        [listener setShow:^(){
            [AdsClass adDisplayed:[self class] forType:type];
            if([tag isEqualToString:@"notification"])
                [CBNewsfeed showNotificationUI];
            else
                [CBNewsfeed showNewsfeedUI];
        }];
        [listener setDestroy:^(){}];
        [listener setHide:^(){
            if([tag isEqualToString:@"notification"])
                [CBNewsfeed closeNotificationUI];
            else
                [CBNewsfeed closeNewsfeedUI];
        }];
        [self.mngr set:listener forType:type withListener:listener];
        [self.mngr load:type];
        [AdsClass adReceived:[self class] forType:type];
    }
    else if ([type isEqualToString:@"v4vc"]) {
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
        [listener setShow:^(){
            [AdsClass adDisplayed:[self class] forType:type];
            [Chartboost showRewardedVideo:tag];
        }];
        [listener setDestroy:^(){}];
        [listener setHide:^(){}];
        [self.mngr set:listener forType:type withListener:listener];
        if([Chartboost hasRewardedVideo:tag]){
            [self.mngr load:type];
            [AdsClass adReceived:[self class] forType:type];
        }
        else
            [Chartboost cacheRewardedVideo:tag];
    }
    else
    {
         [AdsClass adError:[self class] with:[NSString stringWithFormat:@"Unknown type: %@", type]];
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

- (BOOL)shouldRequestInterstitial:(NSString *)location{
    return YES;
}

- (BOOL)shouldDisplayInterstitial:(NSString *)location{
    return YES;
}

- (void)didCacheInterstitial:(NSString *)location{
    if([self.mngr get:@"interstitial"] != nil && ![self.mngr isLoaded:@"interstitial"]){
        [AdsClass adReceived:[self class] forType:@"interstitial"];
        [self.mngr load:@"interstitial"];
    }
}

- (void)didFailToLoadInterstitial:(NSString *)location{
    [AdsClass adFailed:[self class] with:@"Failed to receive ads" forType:@"interstitial"];
    [self.mngr reset:@"interstitial"];
}

- (void)didDismissInterstitial:(NSString *)location{
    [AdsClass adDismissed:[self class] forType:@"interstitial"];
}

- (void)didCloseInterstitial:(NSString *)location{
    [AdsClass adActionEnd:[self class] forType:@"interstitial"];
}

- (void)didClickInterstitial:(NSString *)location{
    [AdsClass adActionBegin:[self class] forType:@"interstitial"];
}


- (void)didCacheMoreApps:(NSString *)location{
    if([self.mngr get:@"moreapps"] != nil && ![self.mngr isLoaded:@"moreapps"]){
        [AdsClass adReceived:[self class] forType:@"moreapps"];
        [self.mngr load:@"moreapps"];
    }

}

- (BOOL)shouldDisplayMoreApps:(NSString *)location{
    return YES;
}

- (void)didFailToLoadMoreApps:(CBLocation)location withError:(CBLoadError)error{
    [AdsClass adFailed:[self class] with:@"Failed to receive ads" forType:@"moreapps"];
    [self.mngr reset:@"moreapps"];
}

- (void)didDismissMoreApps:(NSString *)location{
    [AdsClass adDismissed:[self class] forType:@"moreapps"];
}

- (void)didCloseMoreApps:(NSString *)location{
    [AdsClass adActionEnd:[self class] forType:@"moreapps"];
}

- (void)didClickMoreApps:(NSString *)location{
    [AdsClass adActionBegin:[self class] forType:@"moreapps"];
}

// Called before a rewarded video will be displayed on the screen.
- (BOOL)shouldDisplayRewardedVideo:(CBLocation)location{
    return YES;
}

// Called after a rewarded video has been loaded from the Chartboost API
// servers and cached locally.
- (void)didCacheRewardedVideo:(CBLocation)location{
    if([self.mngr get:@"v4vc"] != nil && ![self.mngr isLoaded:@"v4vc"]){
        [AdsClass adReceived:[self class] forType:@"v4vc"];
        [self.mngr load:@"v4vc"];
    }
}

// Called after a rewarded video has attempted to load from the Chartboost API
// servers but failed.
- (void)didFailToLoadRewardedVideo:(CBLocation)location withError:(CBLoadError)error{
    [AdsClass adFailed:[self class] with:@"Failed to receive ads" forType:@"v4vc"];
    [self.mngr reset:@"v4vc"];
}

// Called after a rewarded video has been dismissed.
- (void)didDismissRewardedVideo:(CBLocation)location{
    [AdsClass adDismissed:[self class] forType:@"v4vc"];
}

// Called after a rewarded video has been clicked.
- (void)didClickRewardedVideo:(CBLocation)location{
    [AdsClass adActionBegin:[self class] forType:@"v4vc"];
}

// Called after a rewarded video has been viewed completely and user is eligible for reward.
- (void)didCompleteRewardedVideo:(CBLocation)location withReward:(int)reward{
    [AdsClass adActionEnd:[self class] forType:@"v4vc"];
}

@end
