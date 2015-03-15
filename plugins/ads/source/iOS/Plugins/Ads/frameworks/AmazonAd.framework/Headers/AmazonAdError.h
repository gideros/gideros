//
//  AmazonAdError.h
//  AmazonMobileAdsSDK
//
//  Copyright (c) 2012-2014 Amazon.com. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef enum {
    AmazonAdErrorRequest,           // Invalid Request. Example : "An invalid request was sent".
    AmazonAdErrorNoFill,            // No ad returned from the server. Example : "Ad not found".
    AmazonAdErrorInternalServer,    // Internal server error. Example : "Failed to load configuration".
    AmazonAdErrorNetworkConnection, // Network Connection error
    AmazonAdErrorReserved
} AmazonAdErrorCode;

@interface AmazonAdError : NSObject

@property (nonatomic, readonly) AmazonAdErrorCode errorCode;
@property (nonatomic, strong, readonly) NSString *errorDescription; 

@end
