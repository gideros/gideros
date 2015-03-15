//
//  AdsManager.h
//  Ads
//
//  Created by Arturs Sosins on 3/15/14.
//  Copyright (c) 2014 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AdsState.h"

@interface AdsManager : NSObject
@property(nonatomic, retain)NSMutableDictionary *adViews_;

-(id)init;
-(NSObject*)get:(NSString*)type;
-(AdsState*)getState:(NSString*)type;
-(void)set:(NSObject*)object forType:(NSString*)type;
-(void)set:(NSObject*)object forType:(NSString*)type withListener:(AdsStateChangeListener*)listener;
-(void)setObject:(NSObject*)object forType:(NSString*)type;
-(void)setListener:(AdsStateChangeListener*)listener forType:(NSString*)type;
-(void)setAutoKill:(BOOL)autokill forType:(NSString*)type;
-(void)remove:(NSString*)type;
-(void)hide:(NSString*)type;
-(void)destroy;
-(void)reset:(NSString*)type;
-(void)reset:(NSString*)type andKill:(BOOL)kill;
-(void)show:(NSString*)type;
-(void)load:(NSString*)type;
-(BOOL)isReady:(NSString*)type;
-(BOOL)isLoaded:(NSString*)type;
@end
