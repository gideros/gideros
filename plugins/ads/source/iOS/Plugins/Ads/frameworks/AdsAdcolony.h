//
//  AdsAdcolony.h
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AdsProtocol.h"
#import <AdColony/AdColony.h>
#import "AdsManager.h"

@interface AdsAdcolony : NSObject <AdsProtocol>

@property (nonatomic, retain) NSString *appKey;
@property (nonatomic, retain) NSString *videoZone1;
@property (nonatomic, retain) AdsManager *mngr;

@end

@interface AdsAdcolonyListener : NSObject <AdColonyDelegate, AdColonyAdDelegate>
@property (nonatomic, retain) AdsState *state;
@property (nonatomic, retain) AdsAdcolony *instance;

-(id)init:(AdsState*)state with:(AdsAdcolony*)instance;
@end
