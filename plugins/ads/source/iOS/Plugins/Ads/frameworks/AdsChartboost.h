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
#import "AdsManager.h"

#ifdef HAS_NEWSFEED
#import <Chartboost/CBNewsfeed.h>
@interface AdsChartboost : NSObject <AdsProtocol, ChartboostDelegate,CBNewsfeedDelegate>
#else
@interface AdsChartboost : NSObject <AdsProtocol, ChartboostDelegate>
#endif

@property (nonatomic, retain) AdsManager *mngr;
@end
