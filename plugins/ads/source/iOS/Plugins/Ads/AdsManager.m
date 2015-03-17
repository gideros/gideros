//
//  AdsManager.m
//  Ads
//
//  Created by Arturs Sosins on 3/15/14.
//  Copyright (c) 2014 Gideros Mobile. All rights reserved.
//

#import "AdsManager.h"

@implementation AdsManager
-(id)init{
    self.adViews_ = [NSMutableDictionary dictionary];
    return self;
}
-(NSObject*)get:(NSString*)type{
    AdsState *state = [self.adViews_ objectForKey:type];
    if(state != nil)
        return [state getObject];
    return nil;
}
-(AdsState*)getState:(NSString*)type{
    return [self.adViews_ objectForKey:type];
}
-(void)set:(NSObject*)object forType:(NSString*)type{
    AdsState *state = [[AdsState alloc] init:object withType:type];
    [self.adViews_ setObject:state forKey:type];
}
-(void)set:(NSObject*)object forType:(NSString*)type withListener:(AdsStateChangeListener*)listener{
    AdsState *state = [[AdsState alloc] init:object withType:type];
    [state setListener:listener];
    [self.adViews_ setObject:state forKey:type];
}
-(void)setObject:(NSObject*)object forType:(NSString*)type{
    AdsState *state = [self.adViews_ objectForKey:type];
    if(state != nil)
        [state setObject:object];
}
-(void)setListener:(AdsStateChangeListener*)listener forType:(NSString*)type{
    AdsState *state = [self.adViews_ objectForKey:type];
    if(state != nil)
        [state setListener:listener];
}
-(void)setAutoKill:(BOOL)autokill forType:(NSString*)type{
    AdsState *state = [self.adViews_ objectForKey:type];
    if(state != nil)
        [state setAutokill:autokill];
}
-(void)remove:(NSString*)type{
    AdsState *state = [self.adViews_ objectForKey:type];
    if(state != nil)
        [state destruct];
    [self.adViews_ removeObjectForKey:type];
}
-(void)hide:(NSString*)type{
    AdsState *state = [self.adViews_ objectForKey:type];
    if(state != nil)
        [state hide];
}
-(void)destroy{
    for(NSString *key in self.adViews_)
    {
        AdsState *state = [self.adViews_ objectForKey:key];
        [state destroy];
        [state destruct];
        [state release];
    }
}

-(void)reset:(NSString*)type{
    [self reset:type andKill:true];
}
-(void)reset:(NSString*)type andKill:(BOOL)kill{
    AdsState *state = [self.adViews_ objectForKey:type];
    if(state != nil)
        [state reset];
    if(kill)
        [self remove:type];
}

-(void)show:(NSString*)type{
    AdsState *state = [self.adViews_ objectForKey:type];
    if(state != nil)
        [state show];
}
-(void)load:(NSString*)type{
    AdsState *state = [self.adViews_ objectForKey:type];
    if(state != nil)
        [state load];
}
-(BOOL)isReady:(NSString*)type{
    AdsState *state = [self.adViews_ objectForKey:type];
    if(state != nil)
        return[state isReady];
    return false;
}
-(BOOL)isLoaded:(NSString*)type{
    AdsState *state = [self.adViews_ objectForKey:type];
    if(state != nil)
        return[state isLoaded];
    return false;
}
@end
