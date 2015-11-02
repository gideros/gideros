//
//  AdsAdcolony.h
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AdsProtocol.h"
#import <VungleSDK/VungleSDK.h>
#import "AdsManager.h"

@interface AdsVungle : NSObject <AdsProtocol, VungleSDKDelegate>
@property(nonatomic) BOOL hasVideo;
@property(nonatomic) BOOL hasV4vc;
@property (nonatomic, retain) AdsManager *mngr;

-(IBAction)showAd;
-(IBAction)showIncentivizedAd;
@end
