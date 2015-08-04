//
//  HZNativeAd.h
//  Heyzap
//
//  Created by Maximilian Tagher on 9/8/14.
//  Copyright (c) 2014 Heyzap. All rights reserved.
//

#import <Foundation/Foundation.h>
@class HZNativeAdCollection;

/**
 *  Use this class to fetch native ads from Heyzap. See `HZNativeAdCollection` and `HZNativeAd` for details.
 */
@interface HZNativeAdController : NSObject

/**
 *  Use this method to fetch native ads from heyzap. 
 
 *  If you were to e.g. display one ad after each level, you should ideally initially fetch several ads (say 10) and work through that list over time rather than do 10 separate ad requests. This cuts down on network requests, saving battery life. Additionally, when fetching several ads at once, all of those ads will be unique--doing separate fetches increases the likelihood you'll have duplicate ads.
 *
 *  @param numberOfAds The number of ads you'd like to receive from the server. For example, if you're displaying a list of ads in a `UITableView`, you might fetch 10 ads and use one for each row.
 *  @param tag         An identifier for the location/context of the ad which you can use to disable the ad from your dashboard. This value can be `nil`.
 *  @param completion  A block called upon success or failure at fetching ads from our server. If `error` is non-nil, we failed to fetch any ads. Otherwise, `error` will be `nil` and `collection` can be used to access a list of `HZNativeAd` objects. `completion` must not be `nil`.
 
 *  @exception NSInternalInconsistencyException if `numberOfAds == 0`
 *  @exception NSInternalInconsistencyException if `completion` is `nil`.
 */
+ (void)fetchAds:(NSUInteger)numberOfAds
             tag:(NSString *)tag
      completion:(void (^)(NSError *error, HZNativeAdCollection *collection))completion;

@end
