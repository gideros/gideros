//
//  ALAdUpdateDelegate.h
//  sdk
//
//  Created by David Anderson on 10/1/12.
//
//

#import <Foundation/Foundation.h>
#import "ALNullabilityAnnotations.h"

@protocol ALAdUpdateObserver <NSObject>

-(void) adService: (alnonnull ALAdService *) adService didUpdateAd: (alnullable ALAd *) ad;
-(BOOL) canAcceptUpdate;

@end
