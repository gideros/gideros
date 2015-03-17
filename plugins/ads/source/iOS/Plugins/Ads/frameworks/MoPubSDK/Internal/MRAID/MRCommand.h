//
//  MRCommand.h
//  MoPub
//
//  Created by Andrew He on 12/19/11.
//  Copyright (c) 2011 MoPub, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

@class MRCommand;

@protocol MRCommandDelegate <NSObject>

- (void)mrCommand:(MRCommand *)command createCalendarEventWithParams:(NSDictionary *)params;
- (void)mrCommand:(MRCommand *)command playVideoWithURL:(NSURL *)url;
- (void)mrCommand:(MRCommand *)command storePictureWithURL:(NSURL *)url;
- (void)mrCommand:(MRCommand *)command shouldUseCustomClose:(BOOL)useCustomClose;
- (void)mrCommand:(MRCommand *)command openURL:(NSURL *)url;
- (void)mrCommand:(MRCommand *)command expandWithParams:(NSDictionary *)params;
- (void)mrCommandClose:(MRCommand *)command;

@end

////////////////////////////////////////////////////////////////////////////////////////////////////

@interface MRCommand : NSObject

@property (nonatomic, assign) id<MRCommandDelegate> delegate;

+ (id)commandForString:(NSString *)string;

// returns YES by default for user safety
- (BOOL)requiresUserInteractionForPlacementType:(NSUInteger)placementType;

- (BOOL)executeWithParams:(NSDictionary *)params;

@end

////////////////////////////////////////////////////////////////////////////////////////////////////

@interface MRCloseCommand : MRCommand

@end

////////////////////////////////////////////////////////////////////////////////////////////////////

@interface MRExpandCommand : MRCommand

@end

////////////////////////////////////////////////////////////////////////////////////////////////////

@interface MRUseCustomCloseCommand : MRCommand

@end

////////////////////////////////////////////////////////////////////////////////////////////////////

@interface MROpenCommand : MRCommand

@end

////////////////////////////////////////////////////////////////////////////////////////////////////

@interface MRCreateCalendarEventCommand : MRCommand

@end

////////////////////////////////////////////////////////////////////////////////////////////////////

@interface MRPlayVideoCommand : MRCommand

@end

////////////////////////////////////////////////////////////////////////////////////////////////////

@interface MRStorePictureCommand : MRCommand

@end
