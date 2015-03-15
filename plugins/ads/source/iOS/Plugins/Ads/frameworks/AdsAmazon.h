//
//  AdsAdmob.h
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AdsProtocol.h"
#import <AmazonAd/AmazonAdRegistration.h>
#import <AmazonAd/AmazonAdView.h>
#import <AmazonAd/AmazonAdInterstitial.h>
#import <AmazonAd/AmazonAdOptions.h>
#import <AmazonAd/AmazonAdError.h>
#import "AdsManager.h"

@interface AdsAmazon : NSObject <AdsProtocol>
@property(nonatomic) CGSize currentType;
@property(nonatomic, retain) NSString *currentSize;
@property(nonatomic, retain) NSString *appKey;
@property(nonatomic) BOOL test;
@property (nonatomic, retain) AdsManager *mngr;
@end

@interface AdsAmazonListener : NSObject <AmazonAdViewDelegate, AmazonAdInterstitialDelegate>
@property (nonatomic, retain) AdsState *state;
@property (nonatomic, retain) AdsAmazon *instance;
-(id)init:(AdsState*)state with:(AdsAmazon*)instance;
@end
