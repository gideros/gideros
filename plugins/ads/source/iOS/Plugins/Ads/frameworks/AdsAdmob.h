//
//  AdsAdmob.h
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AdsProtocol.h"
@import GoogleMobileAds;
#import <AdSupport/ASIdentifierManager.h>
#import <CommonCrypto/CommonDigest.h>
#import "AdsManager.h"

@interface AdsAdmob : NSObject <AdsProtocol, GADInterstitialDelegate>
@property(nonatomic, assign) const GADAdSize *currentType;
@property(nonatomic, retain) NSString *currentSize;
@property(nonatomic, retain) NSString *appKey;
@property(nonatomic, retain) NSString *testID;
@property (nonatomic, retain) AdsManager *mngr;
@end

@interface AdsAdmobListener : NSObject <GADBannerViewDelegate>
@property (nonatomic, retain) AdsState *state;
@property (nonatomic, retain) AdsAdmob *instance;
-(id)init:(AdsState*)state with:(AdsAdmob*)instance;
@end
