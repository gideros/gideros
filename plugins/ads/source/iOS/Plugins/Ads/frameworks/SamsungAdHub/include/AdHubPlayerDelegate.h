//
// AdHubPlayerDelegate.h
// SamsungAdHub
//
// Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>

@class SADPlayerConnection;
@protocol AdHubPlayerDelegate <NSObject>
@optional

/**
@brief Is is called when video player has error.
*/
- (void)adHubPlayerError;

/**
@brief Is is called when video player is closed.
*/
- (void)adHubPlayerClose;
@end

