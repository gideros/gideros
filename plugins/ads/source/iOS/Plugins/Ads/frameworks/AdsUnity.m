//
//  AdsAdcolony.m
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//
#include "gideros.h"
#import "AdsUnity.h"
#import "AdsClass.h"


@implementation AdsUnity

-(id)init{
    self.mngr = [[AdsManager alloc] init];
    self.v4vcMap=YES;
    return self;
}

-(void)destroy{
    [self.mngr destroy];
    [self.mngr release];
    self.mngr = nil;
}

-(void)setKey:(NSMutableArray*)parameters{
    [UnityAds initialize:[parameters objectAtIndex:0] delegate: self];
}

- (NSString *) mapType:(NSString *)type
{
		if ([type isEqual:@"rewardedVideo"])
			_v4vcMap=NO;
		if (_v4vcMap&&[type isEqual:@"v4vc"])
			return @"rewardedVideo";
		return type;
}
	
- (NSString *) unmapType:(NSString *)type
{
		if (_v4vcMap&&[type isEqual:@"rewardedVideo"])
			return @"v4vc";
		return type;		
}

#pragma mark - UnityAds Delegate

- (void)unityAdsReady:(NSString *)placementId {
    [AdsClass adReceived:[self class] forType:[self unmapType:placementId]];
}

- (void)unityAdsDidStart:(NSString *)placementId {
    NSLog(@"An ad is about to be played!");
    [AdsClass adDisplayed:[self class] forType:[self unmapType:placementId]];
}

- (void)unityAdsDidError:(UnityAdsError)error withMessage:(NSString *)message {
    NSLog(@"Unity Ads Error occured:%@",message);
}

- (void)unityAdsDidFinish:(NSString *)placementId withFinishState:(UnityAdsFinishState)state {
    if(state==kUnityAdsFinishStateCompleted){
        [AdsClass adActionEnd:[self class] forType:[self unmapType:placementId]];
    } else if(state==kUnityAdsFinishStateSkipped) {
        [AdsClass adDismissed:[self class] forType:[self unmapType:placementId]];
    } else if(state==kUnityAdsFinishStateError) {
        [AdsClass adFailed:[self class] with:@"" forType:[self unmapType:placementId]];
    }
}


-(void)loadAd:(NSMutableArray*)parameters{
    NSString *type = [parameters objectAtIndex:0];

        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
            [listener setShow:^(){
                [UnityAds show:[AdsClass getRootViewController] placementId:[self mapType:type]];
            }];
            [listener setDestroy:^(){}];
            [listener setHide:^(){}];
            [self.mngr set:type forType:type withListener:listener];
            [self.mngr load:type];
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
