//
//  AdsHeyzap.h
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AdsProtocol.h"
#import <HeyzapAds/HeyzapAds.h>
#import "AdsManager.h"

@interface AdsHeyzap : NSObject <AdsProtocol, HZAdsDelegate, HZIncentivizedAdDelegate>
@property(nonatomic) BOOL hasInterstitial;
@property(nonatomic) BOOL hasVideo;
@property(nonatomic) BOOL hasV4vc;
@property (nonatomic, retain) AdsManager *mngr;
@end
