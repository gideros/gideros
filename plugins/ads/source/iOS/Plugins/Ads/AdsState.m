//
//  AdsState.m
//  Ads
//
//  Created by Arturs Sosins on 3/15/14.
//  Copyright (c) 2014 Gideros Mobile. All rights reserved.
//

#import "AdsState.h"

@implementation AdsState
-(id)init:(NSObject*)ad withType:(NSString*)type{
    self.ad_ = ad;
    self.type_ = type;
    self.loaded = false;
    self.showed = false;
    self.autokill_ = true;
    return self;
}
-(void)destruct{
    self.ad_ = nil;
    self.type_ = nil;
    if(self.listener_ != nil)
    {
        [self.listener_ destroy];
        [self.listener_ release];
    }
    self.listener_ = nil;
}
-(void)setObject:(NSObject*)ad{
    self.ad_ = ad;
}
-(void)setListener:(AdsStateChangeListener*)listener{
    self.listener_ = listener;
}
-(void)setAutokill:(BOOL)autokill{
    self.autokill_ = autokill;
}
-(NSObject*)getObject{
    return self.ad_;
}
-(NSString*)getType{
    return self.type_;
}
-(void)load{
    self.loaded = true;
    [self checkAction];
}
-(void)show{
    self.showed = true;
    [self checkAction];
}
-(BOOL)isReady{
    return (self.loaded && self.showed);
}
-(BOOL)isLoaded{
    return self.loaded;
}
-(void)reset{
    [self reset:true];
}
-(void)reset:(BOOL)reset{
    [self hide];
    if(reset)
    {
        self.ad_ = nil;
    }
    self.loaded = false;
    self.showed = false;
}
-(void)destroy{
    if(self.listener_ != nil)
    {
        @try {
            [self.listener_ onDestroy];
        }
        @catch (NSException * e) {
            NSLog(@"Exception: %@", e);
        }
    }
}
-(void)hide{
    self.showed = false;
    if(self.listener_ != nil)
    {
        @try {
            [self.listener_ onHide];
        }
        @catch (NSException * e) {
            NSLog(@"Exception: %@", e);
        }
    }

}
-(BOOL)checkAction{
    if([self isReady] && self.ad_ != nil && self.listener_ != nil)
    {
        @try{
            [self.listener_ onShow];
        }
        @catch(NSException *e){
            NSLog(@"Exception: %@", e);
        }
        
        if(self.autokill_)
            [self reset:true];
    }
}
@end
