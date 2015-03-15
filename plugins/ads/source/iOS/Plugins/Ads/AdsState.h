//
//  AdsState.h
//  Ads
//
//  Created by Arturs Sosins on 3/15/14.
//  Copyright (c) 2014 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AdsStateChangeListener.h"

@interface AdsState : NSObject
@property(nonatomic, retain) AdsStateChangeListener *listener_;
@property(nonatomic, retain) NSObject *ad_;
@property(nonatomic, retain) NSString *type_;
@property(nonatomic) BOOL loaded;
@property(nonatomic) BOOL showed;
@property(nonatomic) BOOL autokill_;

-(id)init:(NSObject*)ad withType:(NSString*)type;
-(void)destruct;
-(void)setObject:(NSObject*)ad;
-(void)setListener:(AdsStateChangeListener*)listener;
-(void)setAutokill:(BOOL)autokill;
-(NSObject*)getObject;
-(NSString*)getType;
-(void)load;
-(void)show;
-(BOOL)isReady;
-(BOOL)isLoaded;
-(void)reset;
-(void)reset:(BOOL)reset;
-(void)destroy;
-(void)hide;
-(BOOL)checkAction;
@end
