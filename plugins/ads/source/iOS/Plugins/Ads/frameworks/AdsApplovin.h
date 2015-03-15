//
//  AdsAdmob.h
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AdsProtocol.h"
#import "ALSdk.h"
#import "ALAdService.h"
#import "ALAd.h"
#import "ALInterstitialAd.h"
#import "ALAdView.h"
#import "ALAdLoadDelegate.h"
#import "AdsManager.h"

@interface AdsApplovin : NSObject <AdsProtocol>
@property(nonatomic, assign) ALAdView *view_;
@property(nonatomic, retain) NSString *appKey;
@property(nonatomic, retain) NSString *curType;
@property (nonatomic, retain) AdsManager *mngr;
@end

@interface AdsApplovinListener : NSObject <ALAdLoadDelegate, ALAdDisplayDelegate>
@property (nonatomic, retain) AdsState *state;
@property (nonatomic, retain) AdsApplovin *instance;
-(id)init:(AdsState*)state with:(AdsApplovin*)instance;
@end