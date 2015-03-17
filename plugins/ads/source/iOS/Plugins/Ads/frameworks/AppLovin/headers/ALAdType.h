//
//  ALAdType.h
//  sdk
//
//  Created by Matt Szaro on 10/1/13.
//
//

#import <Foundation/Foundation.h>


/**
 *  This class represents the behavior of an ad.
 */

@interface ALAdType : NSObject <NSCopying>

/**
 *  @name Ad Type Identification
 */

/**
 *  String representing the name of this ad type.
 */
@property (strong, readonly) NSString* label;

/**
 *  @name Supported Ad Type Singletons
 */

/**
 *  Represents a standard advertisement.
 *
 *  @return ALAdType representing a standard advertisement.
 */
+(ALAdType*) typeRegular;

/**
 *  Represents a rewarded video.
 *
 *  Typically, you'll award your users coins for viewing this type of ad.
 *
 *  @return ALAdType representing a rewarded video.
 */
+(ALAdType*) typeIncentivized;

/**
 *  Retrieve an <code>NSArray</code> of all available ad size singleton instances.
 *
 *  @return <code>[NSArray arrayWithObjects: [ALAdType typeRegular], [ALAdType typeIncentivized], nil];</code>
 */
+(NSArray*) allTypes;


// Avoid these methods unless specifically necessary.
-(instancetype) initWithLabel: (NSString *) label;
+(ALAdType *) adTypeFromString: (NSString*) adType;

@end
