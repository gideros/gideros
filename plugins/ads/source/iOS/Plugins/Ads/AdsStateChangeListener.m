//
//  AdsStateChangeListener.m
//  Ads
//
//  Created by Arturs Sosins on 3/15/14.
//  Copyright (c) 2014 Gideros Mobile. All rights reserved.
//

#import "AdsStateChangeListener.h"

@implementation AdsStateChangeListener
-(id)init{
    return self;
}
-(void) destroy{
     // Clean up.
    if(_showHandler != nil)
    {
        [_showHandler release];
        _showHandler = nil;
    }
    if(_hideHandler != nil)
    {
        [_hideHandler release];
        _hideHandler = nil;
    }
    if(_destroyHandler != nil)
    {
        [_destroyHandler release];
        _destroyHandler = nil;
    }
}
- (void) setShow:(void (^)())handler
{
    _showHandler = [handler copy];
}

- (void) setDestroy:(void (^)())handler
{
    _destroyHandler = [handler copy];
}

- (void) setHide:(void (^)())handler
{
    _hideHandler = [handler copy];
}
- (void) onShow
{
    if(_showHandler != nil)
        _showHandler();
}

- (void) onDestroy
{
    if(_destroyHandler != nil)
        _destroyHandler();
}

- (void) onHide
{
    if(_hideHandler != nil)
        _hideHandler();
}
@end
