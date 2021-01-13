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
    self.mngr = nil;
    [[VungleSDK sharedSDK] setDelegate:nil];
}

-(void)setKey:(NSMutableArray*)parameters{
    VungleSDK *sdk = [VungleSDK sharedSDK];
    NSString *user = @"Player";
    if([parameters count] > 1)
        user = [parameters objectAtIndex:1];
    NSArray *placements=NULL;
    if([parameters count] > 2) {
        NSRange placementsRange;
        placementsRange.location = 2;
        placementsRange.length = [parameters count] -2;
        placements=[parameters subarrayWithRange:placementsRange];
    }
    else
    {
        placements=@[ @"Default" ];
    }
    
    self.user=user;
    NSError *error=NULL;
    [sdk setDelegate:self];
    [sdk startWithAppId:[parameters objectAtIndex:0] placements:placements error:&error];
    if (error) {
        NSLog(@"Error encountered initializing vungle: %@", error);
        [AdsClass adFailed:[self class] with:error.description forType:@""];
    }
}

#pragma mark - VungleSDK Delegate

- (void)vungleSDKDidInitialize
{
    
}

- (void)vungleAdPlayabilityUpdate:(BOOL)isAdPlayable placementID:(NSString *)placementID {
    if (isAdPlayable) {
        NSLog(@"An ad is available for playback");
        [self.mngr load:placementID];
        [AdsClass adReceived:[self class] forType:placementID];

    } else {
        NSLog(@"No ads currently available for playback");
    }
}

- (void)vungleWillShowAdForPlacementID:(nullable NSString *)placementID
{
    NSLog(@"An ad is about to be played!");
    [AdsClass adDisplayed:[self class] forType:placementID];
}

- (void)vungleWillCloseAdWithViewInfo:(VungleViewInfo *)info placementID:(NSString *)placementID
{
        if([info.completedView boolValue]){
            [AdsClass adActionEnd:[self class] forType:placementID];
        } else {
            [AdsClass adDismissed:[self class] forType:placementID];
        }
}

#pragma mark - FirstView Methods

-(IBAction)showAd: (NSString *)placement forUser:(NSString *)user {
    // Grab instance of Vungle SDK
    VungleSDK* sdk = [VungleSDK sharedSDK];
    
    // Dict to set custom ad options
    NSDictionary* options = @{VunglePlayAdOptionKeyUser: user,
                              VunglePlayAdOptionKeyIncentivizedAlertBodyText : @"If the video isn't completed you won't get your reward! Are you sure you want to close early?",
                              VunglePlayAdOptionKeyIncentivizedAlertCloseButtonText : @"Close",
                              VunglePlayAdOptionKeyIncentivizedAlertContinueButtonText : @"Keep Watching",
                              VunglePlayAdOptionKeyIncentivizedAlertTitleText : @"Careful!"};
    
    // Pass in dict of options, play ad
    NSError *error=NULL;
    [sdk playAd:[AdsClass getRootViewController] options:options placementID:placement error:&error];
    if (error) {
        NSLog(@"Error encountered playing ad: %@", error);
        [AdsClass adFailed:[self class] with:error.description forType:placement];
    }
}

-(void)loadAd:(NSMutableArray*)parameters{
    NSString *type = [parameters objectAtIndex:0];
    NSString *user = self.user;
    if([parameters count] > 1)
    {
        user = [parameters objectAtIndex:1];
    }

        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
            [listener setShow:^(){
                [self showAd:type forUser:user];
            }];
            [listener setDestroy:^(){}];
            [listener setHide:^(){}];
            [self.mngr set:type forType:type withListener:listener];
    
    NSError* error=NULL;
    VungleSDK* sdk = [VungleSDK sharedSDK];
    [sdk loadPlacementWithID:type error:&error];
    if (error) {
        NSLog(@"Error encountered loading ad: %@", error);
        [AdsClass adFailed:[self class] with:error.description forType:type];
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
