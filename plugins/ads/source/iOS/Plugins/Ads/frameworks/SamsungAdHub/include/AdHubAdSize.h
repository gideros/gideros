//
// AdHubAdSize.h
// SamsungAdHub
//
// Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

typedef struct AdHubAdSize {
CGSize size;
NSUInteger adType;
} AdHubAdSize;

/**
@brief Banner ad size 300x250
*/
extern AdHubAdSize const kAdHubAdSize_B_300x250;

/**
@brief Banner ad size 320x48
*/
extern AdHubAdSize const kAdHubAdSize_B_320x48;

/**
@brief Banner ad size 320x50
*/
extern AdHubAdSize const kAdHubAdSize_B_320x50;

/**
@brief Banner ad size 640x100
*/
extern AdHubAdSize const kAdHubAdSize_B_640x100;

/**
@brief Banner ad size 728x90
*/
extern AdHubAdSize const kAdHubAdSize_B_728x90;

