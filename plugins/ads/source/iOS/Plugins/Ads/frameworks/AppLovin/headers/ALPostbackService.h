//
//  ALPostbackService.h
//  sdk
//
//  Created by Matt Szaro on 4/15/15.
//
//

#import <Foundation/Foundation.h>
#import "ALNullabilityAnnotations.h"
#import "ALPostbackDelegate.h"

@class ALSdk;

/**
 * Defines an AppLovin service which can be used to dispatch HTTP GET postbacks to arbitrary URLs.
 *
 * While we provide this service primarily as a convenience for native ad tracking URLs, you are welcome to use it for any postbacks you need to dispatch. Postbacks dispatched from this service happen in a asynchronous task.
 */
@interface ALPostbackService : NSObject

/**
 * Dispatch a postback to a given URL.
 *
 * @param targetUrl URL to call via HTTP GET.
 * @param listener Optional listener. May be nil.
 */
-(void) dispatchPostbackAsync: (alnonnull NSURL*) targetURL andNotify: (alnullable id <ALPostbackDelegate>) listener;

@end
