//
//  AdsAdcolony.m
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//
#include "gideros.h"
#import "AdsKiip.h"
#import "AdsClass.h"


@implementation AdsKiip

-(id)init{
    self.mngr = [[AdsManager alloc] init];
    self.popMap = [[NSMutableDictionary alloc] init];
    return self;
}

-(void)destroy{
    [self.mngr destroy];
    self.mngr = nil;
}

-(void)setKey:(NSMutableArray*)parameters{
    Kiip *kiip = [[Kiip alloc] initWithAppKey:@"GIDEROS_KIIP_APP_KEY" andSecret:[parameters objectAtIndex:0]];
    kiip.delegate = self;
    [Kiip setSharedInstance:kiip];
}

#pragma mark - Kiip Delegate

/**
 Tells the delegate that Kiip attempted to start a new session.
 
 @param kiip The Kiip instance that started the session.
 @param poptart A unit to be displayed. May be nil.
 @param error If not nil, an error occured.
 */
- (void) kiip:(Kiip *)kiip didStartSessionWithPoptart:(KPPoptart *)poptart error:(NSError *)error
{
    if (poptart) {
        NSString *type=@"sessionStart";
        [AdsClass adReceived:[self class] forType:type];
        [self.popMap setObject:poptart forKey:type];
        [self.mngr load:type];
    }
}

/**
 Tells the delegate that Kiip attempted to end it's session.
 
 @param kiip The Kiip instance that ended the session.
 @param error If not nil, an error occured.
 */
- (void) kiip:(Kiip *)kiip didEndSessionWithError:(NSError *)error
{
    
}



/**
 Tells the delegate that the user has recieved in-game content.
 
 @param kiip The Kiip instance that indicated the user should receive in-game content.
 @param content The identifier of the content that should be awarded to the user.
 @param quantity The amount of content that should be awarded to the user.
 @param transactionId The unique identifer of this transaction.
 @param signature The signature that can be checked to validate this transaction.
 */
- (void) kiip:(Kiip *)kiip didReceiveContent:(NSString *)content quantity:(int)quantity transactionId:(NSString *)transactionId signature:(NSString *)signature
{
    [AdsClass adRewarded:[self class] forType:content withAmount:quantity];
}

#if 0
/**
 Tells the delegate that a video session has begun.
 
 @param kiip The Kiip instance that has begun playing video.
 */
- (void) kiipVideoPlaybackDidBegin:(Kiip *)kiip
{
    
}

/**
 Tells the delegate that a video session has ended.
 
 @param kiip The Kiip instance that has finished playing video.
 */
- (void) kiipVideoPlaybackDidEnd:(Kiip *)kiip
{
    
}
#endif


/*

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
*/


- (void) willPresentPoptart:(KPPoptart *)poptart
{
    NSSet *ks=[self.popMap keysOfEntriesPassingTest:^BOOL(id  _Nonnull key, id  _Nonnull obj, BOOL * _Nonnull stop) {
        return obj==poptart;
    }];
    if ([ks count]==0)
        return;
    NSString *type=[ks anyObject];
    [AdsClass adDisplayed:[self class] forType:type];
}

- (void) didDismissPoptart:(KPPoptart *)poptart
{
    NSSet *ks=[self.popMap keysOfEntriesPassingTest:^BOOL(id  _Nonnull key, id  _Nonnull obj, BOOL * _Nonnull stop) {
        return obj==poptart;
    }];
    if ([ks count]==0)
        return;
    NSString *type=[ks anyObject];
    [AdsClass adDismissed:[self class] forType:type];
}

-(void)loadAd:(NSMutableArray*)parameters{
    NSString *type = [parameters objectAtIndex:0];

    [[Kiip sharedInstance] saveMoment:type withCompletionHandler:^(KPPoptart *poptart, NSError *error) {
        if (error) {
            [AdsClass adFailed:[self class] with:[error description] forType:type];
        }
        if (poptart) {
            [AdsClass adReceived:[self class] forType:type];
            [self.popMap setObject:poptart forKey:type];
            [self.mngr load:type];
        }
        // handle case with no reward available.
        if (!poptart) {
            [AdsClass adFailed:[self class] with:@"no fill" forType:type];
        }
    }];
    
        AdsStateChangeListener *listener = [[AdsStateChangeListener alloc] init];
            [listener setShow:^(){
                KPPoptart *poptart=[self.popMap objectForKey:type];
                if (poptart)
                {
                    poptart.delegate=self;
                    [poptart show];
                }
            }];
            [listener setDestroy:^(){
                [self.popMap removeObjectForKey:type];
            }];
            [listener setHide:^(){}];
            [self.mngr set:type forType:type withListener:listener];
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
    [[Kiip sharedInstance] setTestMode:TRUE];
}

-(BOOL)checkConsent:(BOOL) underAge
{
	return FALSE;
}

-(UIView*)getView{
    return nil;
}
@end
