//
//  AdsChartboost.h
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AdsProtocol.h"
#import <Chartboost/Chartboost.h>
#import <Chartboost/CBNewsfeed.h>
#import "AdsManager.h"

@interface AdsChartboost : NSObject <AdsProtocol, ChartboostDelegate,CBNewsfeedDelegate>
@property (nonatomic, retain) AdsManager *mngr;
@end
