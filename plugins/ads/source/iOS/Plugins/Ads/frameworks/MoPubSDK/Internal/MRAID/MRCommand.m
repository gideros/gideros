//
//  MRCommand.m
//  MoPub
//
//  Created by Andrew He on 12/19/11.
//  Copyright (c) 2011 MoPub, Inc. All rights reserved.
//

#import "MRCommand.h"
#import "MRAdView.h"
#import "MPGlobal.h"
#import "MPLogging.h"

@implementation MRCommand

@synthesize delegate = _delegate;

+ (NSMutableDictionary *)sharedCommandClassMap
{
    static NSMutableDictionary *sharedMap = nil;

    static dispatch_once_t once;
    dispatch_once(&once, ^{
        sharedMap = [[NSMutableDictionary alloc] init];
    });

    return sharedMap;
}

+ (void)registerCommand:(Class)commandClass
{
    NSMutableDictionary *map = [self sharedCommandClassMap];
    @synchronized(self) {
        [map setValue:commandClass forKey:[commandClass commandType]];
    }
}

+ (NSString *)commandType
{
    return @"BASE_CMD_TYPE";
}

+ (Class)commandClassForString:(NSString *)string
{
    NSMutableDictionary *map = [self sharedCommandClassMap];
    @synchronized(self) {
        return [map objectForKey:string];
    }
}

+ (id)commandForString:(NSString *)string
{
    Class commandClass = [self commandClassForString:string];
    return [[[commandClass alloc] init] autorelease];
}

// return YES by default for user safety
- (BOOL)requiresUserInteractionForPlacementType:(NSUInteger)placementType
{
    return YES;
}

- (BOOL)executeWithParams:(NSDictionary *)params
{
    return YES;
}

- (CGFloat)floatFromParameters:(NSDictionary *)parameters forKey:(NSString *)key
{
    return [self floatFromParameters:parameters forKey:key withDefault:0.0];
}

- (CGFloat)floatFromParameters:(NSDictionary *)parameters forKey:(NSString *)key withDefault:(CGFloat)defaultValue
{
    NSString *stringValue = [parameters valueForKey:key];
    return stringValue ? [stringValue floatValue] : defaultValue;
}

- (BOOL)boolFromParameters:(NSDictionary *)parameters forKey:(NSString *)key
{
    NSString *stringValue = [parameters valueForKey:key];
    return [stringValue isEqualToString:@"true"] || [stringValue isEqualToString:@"1"];
}

- (int)intFromParameters:(NSDictionary *)parameters forKey:(NSString *)key
{
    NSString *stringValue = [parameters valueForKey:key];
    return stringValue ? [stringValue intValue] : -1;
}

- (NSString *)stringFromParameters:(NSDictionary *)parameters forKey:(NSString *)key
{
    NSString *value = [parameters objectForKey:key];
    if (!value || [value isEqual:[NSNull null]]) return nil;

    value = [value stringByTrimmingCharactersInSet:
             [NSCharacterSet whitespaceAndNewlineCharacterSet]];
    if (!value || [value isEqual:[NSNull null]] || value.length == 0) return nil;

    return value;
}

- (NSURL *)urlFromParameters:(NSDictionary *)parameters forKey:(NSString *)key
{
    NSString *value = [self stringFromParameters:parameters forKey:key];
    return [NSURL URLWithString:value];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation MRCloseCommand

+ (void)load
{
    [MRCommand registerCommand:self];
}

+ (NSString *)commandType
{
    return @"close";
}

- (BOOL)executeWithParams:(NSDictionary *)params
{
    [self.delegate mrCommandClose:self];
    return YES;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation MRExpandCommand

+ (void)load
{
    [MRCommand registerCommand:self];
}

+ (NSString *)commandType
{
    return @"expand";
}

- (BOOL)executeWithParams:(NSDictionary *)params
{
    CGRect applicationFrame = MPApplicationFrame();
    CGFloat afWidth = CGRectGetWidth(applicationFrame);
    CGFloat afHeight = CGRectGetHeight(applicationFrame);

    // If the ad has expandProperties, we should use the width and height values specified there.
    CGFloat w = [self floatFromParameters:params forKey:@"w" withDefault:afWidth];
    CGFloat h = [self floatFromParameters:params forKey:@"h" withDefault:afHeight];

    // Constrain the ad to the application frame size.
    if (w > afWidth) w = afWidth;
    if (h > afHeight) h = afHeight;

    // Center the ad within the application frame.
    CGFloat x = applicationFrame.origin.x + floor((afWidth - w) / 2);
    CGFloat y = applicationFrame.origin.y + floor((afHeight - h) / 2);

    NSURL *url = [self urlFromParameters:params forKey:@"url"];

    MPLogDebug(@"Expanding to (%.1f, %.1f, %.1f, %.1f); displaying %@.", x, y, w, h, url);

    CGRect newFrame = CGRectMake(x, y, w, h);

    NSDictionary *expandParams = [NSDictionary dictionaryWithObjectsAndKeys:
                                  NSStringFromCGRect(newFrame), @"expandToFrame",
                                  (url == nil) ? [NSNull null] : url , @"url",
                                  [NSNumber numberWithBool:[self boolFromParameters:params forKey:@"shouldUseCustomClose"]], @"useCustomClose",
                                  [NSNumber numberWithBool:NO], @"isModal",
                                  [NSNumber numberWithBool:[self boolFromParameters:params forKey:@"lockOrientation"]], @"shouldLockOrientation",
                                  nil];

    [self.delegate mrCommand:self expandWithParams:expandParams];

    return YES;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation MRUseCustomCloseCommand

+ (void)load
{
    [MRCommand registerCommand:self];
}

- (BOOL)requiresUserInteractionForPlacementType:(NSUInteger)placementType
{
    return NO;
}

+ (NSString *)commandType
{
    return @"usecustomclose";
}

- (BOOL)executeWithParams:(NSDictionary *)params
{
    [self.delegate mrCommand:self shouldUseCustomClose:[self boolFromParameters:params forKey:@"shouldUseCustomClose"]];

    return YES;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation MROpenCommand

+ (void)load
{
    [MRCommand registerCommand:self];
}

+ (NSString *)commandType
{
    return @"open";
}

- (BOOL)executeWithParams:(NSDictionary *)params
{
    [self.delegate mrCommand:self openURL:[self urlFromParameters:params forKey:@"url"]];

    return YES;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation MRCreateCalendarEventCommand

+ (void)load
{
    [MRCommand registerCommand:self];
}

+ (NSString *)commandType
{
    return @"createCalendarEvent";
}

- (BOOL)executeWithParams:(NSDictionary *)params
{
    [self.delegate mrCommand:self createCalendarEventWithParams:params];

    return YES;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation MRPlayVideoCommand

+ (void)load
{
    [MRCommand registerCommand:self];
}

+ (NSString *)commandType
{
    return @"playVideo";
}

- (BOOL)requiresUserInteractionForPlacementType:(NSUInteger)placementType
{
    // allow interstitials to auto-play video
    return placementType != MRAdViewPlacementTypeInterstitial;
}

- (BOOL)executeWithParams:(NSDictionary *)params
{
    [self.delegate mrCommand:self playVideoWithURL:[self urlFromParameters:params forKey:@"uri"]];

    return YES;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation MRStorePictureCommand

+ (void)load
{
    [MRCommand registerCommand:self];
}

+ (NSString *)commandType
{
    return @"storePicture";
}

- (BOOL)executeWithParams:(NSDictionary *)params
{
    [self.delegate mrCommand:self storePictureWithURL:[self urlFromParameters:params forKey:@"uri"]];

    return YES;
}

@end
