//
//  AdsRevmob.h
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AdsProtocol.h"
#import <RevMobAds/RevMobAds.h>
#import "AdsManager.h"

@interface AdsRevmob : NSObject <AdsProtocol>
@property (nonatomic, retain) AdsManager *mngr;
@property(nonatomic, retain) NSString *currentSize;
@end

@interface AdsRevmobListener : NSObject <RevMobAdsDelegate>
@property (nonatomic, retain) AdsState *state;
@property (nonatomic, retain) AdsRevmob *instance;
-(id)init:(AdsState*)state with:(AdsRevmob*)instance;
@end