//
//  AdsMopub.h
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AdsProtocol.h"
#import "MPAdView.h"
#import "MPInterstitialAdController.h"
#import "AdsManager.h"

@interface AdsMopub : NSObject <AdsProtocol, MPAdViewDelegate, MPInterstitialAdControllerDelegate>
@property (nonatomic, retain) NSString *appId;
@property (nonatomic, retain) NSString *currentSize;
@property (nonatomic, retain) AdsManager *mngr;
@end
