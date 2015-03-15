//
//  AdsProtocol.h
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol AdsProtocol <NSObject>

-(id)init;
-(void)destroy;
-(void)setKey:(NSMutableArray*)parameters;
-(void)loadAd:(NSMutableArray*)parameters;
-(void)showAd:(NSMutableArray*)parameters;
-(void)hideAd:(NSString*)type;
-(void)enableTesting;
-(UIView*)getView;
@end
