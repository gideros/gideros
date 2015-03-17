//
//  AdsIad.h
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AdsProtocol.h"
#import "AdsManager.h"
#include <iAd/iAd.h>

@interface AdsIad :  NSObject <AdsProtocol, ADBannerViewDelegate, ADInterstitialAdDelegate>
@property (nonatomic, retain) AdsManager *mngr;
@property (nonatomic, retain) NSString *curSize;
@end
