//
//  AdsSamsung.h
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AdsProtocol.h"
#import "AdHubBarBannerView.h"
#import "AdHubBoxBannerView.h"
#import "AdHubInterstitial.h"
#import "AdHubInterstitialDelegate.h"
#import "AdHubPlayer.h"
#import "AdHubPlayerDelegate.h"
#import "AdsManager.h"

@interface AdsSamsung : NSObject <AdsProtocol, AdHubInterstitialDelegate, AdHubPlayerDelegate>
@property(nonatomic, retain) NSString *appKey;
@property(nonatomic, retain) NSString *currentSize;
@property (nonatomic, retain) AdsManager *mngr;
@end

@interface AdsSamsungListener : NSObject <AdHubViewDelegate>
@property (nonatomic, retain) AdsState *state;
@property (nonatomic, retain) AdsSamsung *instance;
-(id)init:(AdsState*)state with:(AdsSamsung*)instance;
@end