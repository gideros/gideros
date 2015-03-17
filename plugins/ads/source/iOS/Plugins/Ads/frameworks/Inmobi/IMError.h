//
//  IMError.h
//  InMobi Monetization SDK
//
//  Copyright (c) 2013 InMobi. All rights reserved.
//

#import <Foundation/Foundation.h>

/**
 * Constant Error Domain for all the InMobi specific errors.
 */
extern NSString *const kInMobiErrorDomain;

/**
 * NSError codes for InMobi error domain
 */
typedef enum {
    // The ad request is invalid, refer to localizedDescription for more info.
    kIMErrorInvalidRequest = 0,

    // No ads were returned from the Ad Network
    kIMErrorNoFill,

    // InMobi encountered an Internal error
    kIMErrorInternal,

    // Ad Format Not Supported
    kIMErrorAdFormatNotSupported,

    // Ad request/rendering timed out.
    kIMErrorTimeout,

    // Ad request was cancelled.
    kIMErrorRequestCancelled,

    // Do ad monetization.
    kIMErrorDoMonetization,

    // Do nothing
    kIMErrorDoNothing,

} IMErrorCode;

/**
 * This class represents the error generated due to invalid request parameters
 * or when the request fails to load.
 */
@interface IMError : NSError

@end
