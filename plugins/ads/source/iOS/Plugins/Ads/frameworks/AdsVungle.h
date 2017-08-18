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
@property (nonatomic, retain) AdsManager *mngr;
@property (nonatomic, retain) NSString *user;

-(IBAction)showAd:(NSString*) placement forUser:(NSString *)user;
@end
