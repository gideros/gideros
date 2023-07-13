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

@interface AdsAdmob : NSObject <AdsProtocol>
@property(nonatomic, assign) const GADAdSize currentType;
@property(nonatomic, copy) NSString *currentSize;
@property(nonatomic, copy) NSString *appKey;
@property(nonatomic, copy) NSString *testID;
@property (nonatomic, retain) AdsManager *mngr;
- (const GADAdSize) getAdType:(NSString*)type orientation:(NSString*)orientation;

@end

@interface AdsAdmobFullscreenListener : NSObject <GADFullScreenContentDelegate>
@property (nonatomic, retain) AdsState *state;
@property (nonatomic, retain) AdsAdmob *instance;
@property (nonatomic, copy) NSString *placeId;
@property (nonatomic, retain) NSObject *ad;
-(id)init:(AdsState*)state placeId:(NSString *)placeId ad:(NSObject *)ad with:(AdsAdmob*)instance;
@end

@interface AdsAdmobBannerListener : NSObject <GADBannerViewDelegate>
@property (nonatomic, retain) AdsState *state;
@property (nonatomic, retain) AdsAdmob *instance;
-(id)init:(AdsState*)state with:(AdsAdmob*)instance;
@end
