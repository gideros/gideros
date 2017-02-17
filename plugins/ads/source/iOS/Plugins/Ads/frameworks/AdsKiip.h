//
//  AdsAdcolony.h
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AdsProtocol.h"
#import <KiipSDK/KiipSDK.h>
#import "AdsManager.h"

@interface AdsKiip : NSObject <AdsProtocol, KiipDelegate, KPPoptartDelegate>
@property (nonatomic, retain) AdsManager *mngr;
@property (nonatomic, retain) NSMutableDictionary *popMap;

@end
