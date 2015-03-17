//
//  AdsAdcolony.m
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//
#include "gideros.h"
#import "AdsAdcolony.h"
#import "AdsClass.h"

@implementation AdsAdcolony

-(id)init{
    self.appKey = nil;
    self.videoZone1 = nil;
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
    if([parameters count] > 1)
    {
        self.videoZone1 = [parameters objectAtIndex:1];
    }
    [AdColony configureWithAppID:self.appKey
                         zoneIDs:@[self.videoZone1]
                        delegate:nil
                         logging:NO];
}

-(void)loadAd:(NSMutableArray*)parameters{
    NSString *type = [parameters objectAtIndex:0];
    NSString *zone = nil;
    if([parameters count] > 1)
    {
        zone = [parameters objectAtIndex:1];
    }
    NSString *popup = nil;
    if([parameters count] > 2)
    {
        popup = [parameters objectAtIndex:2];
    }

    if ([type isEqualToString:@"video"] || [type isEqualToString:@"auto"]) {
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
        [listener setShow:^(){
            [AdsClass adDisplayed:[self class] forType:type];
            if(zone == nil)
            {
                [AdColony playVideoAdForZone:self.videoZone1 withDelegate:[[AdsAdcolonyListener alloc] init:[self.mngr getState:type] with:self]];
            }
            else
            {
                [AdColony playVideoAdForZone:zone withDelegate:[[AdsAdcolonyListener alloc] init:[self.mngr getState:type] with:self]];
            }
        }];
        [listener setDestroy:^(){}];
        [listener setHide:^(){}];
        [self.mngr set:type forType:type withListener:listener];
        [self.mngr load:type];
        [AdsClass adReceived:[self class] forType:type];
    }
    else if ([type isEqualToString:@"v4vc"]) {
        if ([popup isEqualToString:@"true"]) {
            if([AdColony isVirtualCurrencyRewardAvailableForZone:zone]) {
                AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
                [listener setShow:^(){
                    [AdsClass adDisplayed:[self class] forType:type];
                    [AdColony playVideoAdForZone:zone withDelegate:[[AdsAdcolonyListener alloc] init:[self.mngr getState:type] with:self] withV4VCPrePopup:YES andV4VCPostPopup:YES];
                }];
                [listener setDestroy:^(){}];
                [listener setHide:^(){}];
                [self.mngr set:type forType:type withListener:listener];
                [self.mngr load:type];
                [AdsClass adReceived:[self class] forType:type];
            }
            else
            {
                [AdsClass adFailed:[self class] with:@"No V4VC" forType:type];
            }
        }
        else{
            if([AdColony isVirtualCurrencyRewardAvailableForZone:zone]) {
                AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
                [listener setShow:^(){
                    [AdsClass adDisplayed:[self class] forType:type];
                    [AdColony playVideoAdForZone:zone withDelegate:[[AdsAdcolonyListener alloc] init:[self.mngr getState:type] with:self] withV4VCPrePopup:NO andV4VCPostPopup:YES];
                }];
                [listener setDestroy:^(){}];
                [listener setHide:^(){}];
                [self.mngr set:type forType:type withListener:listener];
                [self.mngr load:type];
                [AdsClass adReceived:[self class] forType:type];
            }
            else
            {
                [AdsClass adFailed:[self class] with:@"No V4VC" forType:type];
            }
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

@end


@implementation AdsAdcolonyListener

-(id)init:(AdsState*)state with:(AdsAdcolony*)instance{
    self.state = state;
    self.instance = instance;
    return self;
}

- (void) onAdColonyV4VCReward:(BOOL)success currencyName:(NSString*)currencyName currencyAmount:(int)amount inZone:(NSString*)zoneID {
	if (success) {
		[AdsClass adActionEnd:[self.instance class] forType:[self.state getType]];
	}
    else
    {
        [AdsClass adDismissed:[self.instance class] forType:[self.state getType]];
    }
}

- (void) onAdColonyAdStartedInZone:(NSString *)zoneID {
    //[AdsClass adDisplayed:[self.instance class] forType:[self.state getType]];
}

- (void) onAdColonyAdAttemptFinished:(BOOL)shown inZone:(NSString *)zoneID {
    if(shown == YES)
    {
        [AdsClass adDismissed:[self.instance class] forType:[self.state getType]];
    }
    else{
        [AdsClass adFailed:[self.instance class] with:@"No ad available" forType:[self.state getType]];
    }
}

- (void) onAdColonyAdAvailabilityChange:(BOOL)available inZone:(NSString *)zoneID {
 
}


@end
