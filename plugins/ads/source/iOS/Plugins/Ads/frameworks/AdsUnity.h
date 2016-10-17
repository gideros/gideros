//
//  AdsAdcolony.h
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AdsProtocol.h"
#import <UnityAds/UnityAds.h>
#import "AdsManager.h"

@interface AdsUnity : NSObject <AdsProtocol, UnityAdsDelegate>
@property(nonatomic) BOOL v4vcMap;
@property (nonatomic, retain) AdsManager *mngr;
- (NSString *) mapType:(NSString *)type;
- (NSString *) unmapType:(NSString *)type;

@end
