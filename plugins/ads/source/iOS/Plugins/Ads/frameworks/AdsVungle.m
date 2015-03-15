//
//  AdsAdcolony.m
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//
#include "gideros.h"
#import "AdsVungle.h"
#import "AdsClass.h"


@implementation AdsVungle

-(id)init{
    self.mngr = [[AdsManager alloc] init];
    return self;
}

-(void)destroy{
    [self.mngr destroy];
    [self.mngr release];
    self.mngr = nil;
    
    [VGVunglePub stop];
}

-(void)setKey:(NSMutableArray*)parameters{
    [VGVunglePub startWithPubAppID:[parameters objectAtIndex:0]];
    [VGVunglePub setDelegate:self];
}

-(void)loadAd:(NSMutableArray*)parameters{
    NSString *type = [parameters objectAtIndex:0];
    NSString *popup = @"Player";
    if([parameters count] > 1)
    {
        popup = [parameters objectAtIndex:1];
    }

    if ([type isEqualToString:@"video"] || [type isEqualToString:@"auto"]) {
        if([VGVunglePub adIsAvailable])
        {
            AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
            [listener setShow:^(){
                self.hasVideo = true;
                [AdsClass adDisplayed:[self class] forType:type];
                [VGVunglePub playModalAd:[AdsClass getRootViewController] animated:YES];
            }];
            [listener setDestroy:^(){}];
            [listener setHide:^(){}];
            [self.mngr set:type forType:type withListener:listener];
            [self.mngr load:type];
        }
        else{
            [AdsClass adFailed:[self class] with:@"No Video available" forType:type];
        }
    }
    else if ([type isEqualToString:@"v4vc"]) {

        if([VGVunglePub adIsAvailable])
        {
            AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
            [listener setShow:^(){
                self.hasV4vc = true;
                [AdsClass adDisplayed:[self class] forType:type];
                [VGVunglePub playIncentivizedAd:[AdsClass getRootViewController] animated:YES showClose:YES userTag:(NSString *)popup];
            }];
            [listener setDestroy:^(){}];
            [listener setHide:^(){}];
            [self.mngr set:type forType:type withListener:listener];
            [self.mngr load:type];
        }
        else
        {
            [AdsClass adFailed:[self class] with:@"No V4VC" forType:type];
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

- (void)vungleMoviePlayed:(VGPlayData*)playData{
    if([playData playedFull])
    {
        if(self.hasV4vc)
            [AdsClass adActionEnd:[self class] forType:@"v4vc"];
        if(self.hasVideo)
            [AdsClass adActionEnd:[self class] forType:@"video"];
    }
    self.hasV4vc = false;
    self.hasVideo = false;
}
- (void)vungleStatusUpdate:(VGStatusData*)statusData{
    
}
- (void)vungleViewDidDisappear:(UIViewController*)viewController willShowProductView:(BOOL)willShow{
    if(self.hasV4vc)
        [AdsClass adDismissed:[self class] forType:@"v4vc"];
    if(self.hasVideo)
        [AdsClass adDismissed:[self class] forType:@"video"];
}

- (void)vungleViewWillAppear:(UIViewController*)viewController{
    if(self.hasV4vc)
        [AdsClass adReceived:[self class] forType:@"v4vc"];
    if(self.hasVideo)
        [AdsClass adReceived:[self class] forType:@"video"];
}
- (void)vungleAppStoreWillAppear{
    
}
- (void)vungleAppStoreViewDidDisappear{
    
}


@end
