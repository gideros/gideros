//
//  AdsTapfortap.h
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AdsProtocol.h"
#import "TFTTapForTap.h"
#import "AdsManager.h"

@interface AdsTapfortap : NSObject <AdsProtocol, TFTBannerDelegate, TFTInterstitialDelegate, TFTAppWallDelegate>
@property (nonatomic, retain) AdsManager *mngr;
@end
