//
//  AdsStateChangeListener.h
//  Ads
//
//  Created by Arturs Sosins on 3/15/14.
//  Copyright (c) 2014 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface AdsStateChangeListener : NSObject
{
    void (^_showHandler)();
    void (^_destroyHandler)();
    void (^_hideHandler)();
}
-(id)init;
-(void)destroy;
-(void)setShow:(void(^)())handler;
-(void)setDestroy:(void(^)())handler;
-(void)setHide:(void(^)())handler;
-(void)onShow;
-(void)onDestroy;
-(void)onHide;
@end
