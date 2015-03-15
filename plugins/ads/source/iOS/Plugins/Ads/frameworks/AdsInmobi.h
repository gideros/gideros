//
//  AdsInmobi.h
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AdsProtocol.h"
#import "InMobi.h"
#import "IMBanner.h"
#import "IMBannerDelegate.h"
#import "IMInterstitial.h"
#import "IMInterstitialDelegate.h"
#import "IMError.h"
#import "InMobiAnalytics.h"
#import "AdsManager.h"

@interface AdsInmobi : NSObject <AdsProtocol, IMInterstitialDelegate>
@property (nonatomic, assign) NSInteger currentType;
@property (nonatomic, retain) NSString *currentSize;
@property (nonatomic, retain) NSMutableDictionary *sizes;
@property (nonatomic, retain) NSString *appKey;
@property (nonatomic, retain) AdsManager *mngr;
@end

@interface AdsInmobiListener : NSObject <IMBannerDelegate>
@property (nonatomic, retain) AdsState *state;
@property (nonatomic, retain) AdsInmobi *instance;

-(id)init:(AdsState*)state with:(AdsInmobi*)instance;
@end
