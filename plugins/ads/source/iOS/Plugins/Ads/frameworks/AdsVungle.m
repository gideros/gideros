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
    [[VungleSDK sharedSDK] setDelegate:nil];
}

-(void)setKey:(NSMutableArray*)parameters{
    VungleSDK *sdk = [VungleSDK sharedSDK];
    [sdk startWithAppId:[parameters objectAtIndex:0]];
    [[VungleSDK sharedSDK] setDelegate:self];
}

-(void)loadAd:(NSMutableArray*)parameters{
    NSString *type = [parameters objectAtIndex:0];
    NSString *popup = @"Player";
    if([parameters count] > 1)
    {
        popup = [parameters objectAtIndex:1];
    }

    if ([type isEqualToString:@"video"] || [type isEqualToString:@"auto"]) {
        if ([[VungleSDK sharedSDK] isAdPlayable])
        {
            [AdsClass adReceived:[self class] forType:@"video"];
            AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
            [listener setShow:^(){
                self.hasVideo = true;
                [AdsClass adDisplayed:[self class] forType:type];
                VungleSDK* sdk = [VungleSDK sharedSDK];
                NSError *error;
                [sdk playAd:[AdsClass getRootViewController] error:&error];
                if (error) {
                    [AdsClass adFailed:[self class] with:error.description forType:type];
                }
            }];
            [listener setDestroy:^(){}];
            [listener setHide:^(){}];
            [self.mngr set:type forType:type withListener:listener];
            [self.mngr load:type];
        }
        else{
            self.hasVideo = false;
            [AdsClass adFailed:[self class] with:@"No Video available" forType:type];
        }
    }
    else if ([type isEqualToString:@"v4vc"]) {

        if ([[VungleSDK sharedSDK] isAdPlayable])
        {
            [AdsClass adReceived:[self class] forType:@"v4vc"];
            AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
            [listener setShow:^(){
                self.hasV4vc = true;
                [AdsClass adDisplayed:[self class] forType:type];
                // Grab instance of Vungle SDK
                VungleSDK* sdk = [VungleSDK sharedSDK];
                
                // Dict to set custom ad options
                NSDictionary* options = @{VunglePlayAdOptionKeyIncentivized: @YES,
                                          VunglePlayAdOptionKeyIncentivizedAlertBodyText : @"If the video isn't completed you won't get your reward! Are you sure you want to close early?",
                                          VunglePlayAdOptionKeyIncentivizedAlertCloseButtonText : @"Close",
                                          VunglePlayAdOptionKeyIncentivizedAlertContinueButtonText : @"Keep Watching",
                                          VunglePlayAdOptionKeyIncentivizedAlertTitleText : @"Reward!"};
                
                // Pass in dict of options, play ad
                NSError *error;
                [sdk playAd:[AdsClass getRootViewController] withOptions:options error:&error];
                if (error) {
                    self.hasV4vc = false;
                    [AdsClass adFailed:[self class] with:error.description forType:type];
                }
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

- (void)vungleSDKAdPlayableChanged:(BOOL)isAdPlayable {
    if (isAdPlayable) {
        if(self.hasV4vc)
            [AdsClass adReceived:[self class] forType:@"v4vc"];
        if(self.hasVideo)
            [AdsClass adReceived:[self class] forType:@"video"];
    } else {
        if(self.hasV4vc)
            [AdsClass adFailed:[self class] with:@"No Ad Available" forType:@"v4vc"];
        if(self.hasVideo)
            [AdsClass adFailed:[self class] with:@"No Ad Available" forType:@"video"];
    }
}

- (void)vungleSDKwillShowAd {
  
}

- (void) vungleSDKwillCloseAdWithViewInfo:(NSDictionary *)viewInfo willPresentProductSheet:(BOOL)willPresentProductSheet {
    
    if(!willPresentProductSheet)
    {
        NSLog(@"The ad presented was not tapped - the user has returned to the app");
        NSLog(@"ViewInfo Dictionary:");
        for(NSString * key in [viewInfo allKeys]) {
            NSLog(@"%@ : %@", key, [[viewInfo objectForKey:key] description]);
        }
        if([viewInfo valueForKey:@"completedView"]){
            if(self.hasV4vc)
                [AdsClass adActionEnd:[self class] forType:@"v4vc"];
            if(self.hasVideo)
                [AdsClass adActionEnd:[self class] forType:@"video"];

        }
        if(self.hasV4vc)
            [AdsClass adDismissed:[self class] forType:@"v4vc"];
        if(self.hasVideo)
            [AdsClass adDismissed:[self class] forType:@"video"];
        self.hasV4vc = false;
        self.hasVideo = false;
    }
}

- (void)vungleSDKwillCloseProductSheet:(id)productSheet {
    if(self.hasV4vc)
        [AdsClass adDismissed:[self class] forType:@"v4vc"];
    if(self.hasVideo)
        [AdsClass adDismissed:[self class] forType:@"video"];
    
    self.hasV4vc = false;
    self.hasVideo = false;
}
/*
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
}*/



@end
