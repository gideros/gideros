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

#pragma mark - VungleSDK Delegate

- (void)vungleSDKAdPlayableChanged:(BOOL)isAdPlayable {
    if (isAdPlayable) {
        NSLog(@"An ad is available for playback");
        [AdsClass adReceived:[self class] forType:@"True"];

    } else {
        NSLog(@"No ads currently available for playback");
    }
}

- (void)vungleSDKwillShowAd {
    NSLog(@"An ad is about to be played!");
    [AdsClass adDisplayed:[self class] forType:@"video"];

    //Use this delegate method to pause animations, sound, etc.
}

- (void) vungleSDKwillCloseAdWithViewInfo:(NSDictionary *)viewInfo willPresentProductSheet:(BOOL)willPresentProductSheet {
    if (willPresentProductSheet) {
        //In this case we don't want to resume animations and sound, the user hasn't returned to the app yet
        NSLog(@"The ad presented was tapped and the user is now being shown the App Product Sheet");
        NSLog(@"ViewInfo Dictionary:");
        for(NSString * key in [viewInfo allKeys]) {
            NSLog(@"%@ : %@", key, [[viewInfo objectForKey:key] description]);
        }
    } else {
        //In this case the user has declined to download the advertised application and is now returning fully to the main app
        //Animations / Sound / Gameplay can be resumed now
        NSLog(@"The ad presented was not tapped - the user has returned to the app");
        NSLog(@"ViewInfo Dictionary:");
        for(NSString * key in [viewInfo allKeys]) {
            NSLog(@"%@ : %@", key, [[viewInfo objectForKey:key] description]);
        }
        if([[viewInfo valueForKey:@"completedView"] isEqual:[NSNumber numberWithBool:YES]]){
            [AdsClass adActionEnd:[self class] forType:@""];
        } else {
            [AdsClass adDismissed:[self class] forType:@""];
        }
    }
}

- (void)vungleSDKwillCloseProductSheet:(id)productSheet {
    NSLog(@"The user has downloaded an advertised application and is now returning to the main app");
    [AdsClass adActionEnd:[self class] forType:@""];
    //This method can be used to resume animations, sound, etc. if a user was presented a product sheet earlier
}

#pragma mark - FirstView Methods

- (IBAction)showAd
{
    // Play a Vungle ad (with default options)
    VungleSDK* sdk = [VungleSDK sharedSDK];
    NSError *error;
    [sdk playAd:[AdsClass getRootViewController] error:&error];
    if (error) {
        NSLog(@"Error encountered playing ad: %@", error);
        [AdsClass adFailed:[self class] with:error.description forType:@""];
    }
}


-(IBAction)showIncentivizedAd{
    // Grab instance of Vungle SDK
    VungleSDK* sdk = [VungleSDK sharedSDK];
    
    // Dict to set custom ad options
    NSDictionary* options = @{VunglePlayAdOptionKeyIncentivized: @YES,
                              VunglePlayAdOptionKeyIncentivizedAlertBodyText : @"If the video isn't completed you won't get your reward! Are you sure you want to close early?",
                              VunglePlayAdOptionKeyIncentivizedAlertCloseButtonText : @"Close",
                              VunglePlayAdOptionKeyIncentivizedAlertContinueButtonText : @"Keep Watching",
                              VunglePlayAdOptionKeyIncentivizedAlertTitleText : @"Careful!"};
    
    // Pass in dict of options, play ad
    NSError *error;
    [sdk playAd:[AdsClass getRootViewController] withOptions:options error:&error];
    if (error) {
        NSLog(@"Error encountered playing ad: %@", error);
        [AdsClass adFailed:[self class] with:error.description forType:@""];
    }
}




-(void)loadAd:(NSMutableArray*)parameters{
    NSString *type = [parameters objectAtIndex:0];
    NSString *popup = @"Player";
    if([parameters count] > 1)
    {
        popup = [parameters objectAtIndex:1];
    }

    if ([type isEqualToString:@"video"] || [type isEqualToString:@"auto"]) {
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
            [listener setShow:^(){
                [self showAd];
            }];
            [listener setDestroy:^(){}];
            [listener setHide:^(){}];
            [self.mngr set:type forType:type withListener:listener];
            [self.mngr load:type];
    } else if ([type isEqualToString:@"v4vc"]) {
            AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
            [listener setShow:^(){
                [self showIncentivizedAd];
            }];
            [listener setDestroy:^(){}];
            [listener setHide:^(){}];
            [self.mngr set:type forType:type withListener:listener];
            [self.mngr load:type];
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
