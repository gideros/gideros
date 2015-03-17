//
//  GNotification.h
//  NotificatinTest
//
//  Created by Arturs Sosins on 3/13/13.
//  Copyright (c) 2013 Arturs Sosins. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "NotificationClass.h"

@interface GNotification : NSObject
@property int nid;
@property int number;
@property bool isDispatched;
@property (nonatomic, assign) NSString *title;
@property (nonatomic, assign) NSString *body;
@property (nonatomic, assign) NSString *sound;
@property (nonatomic, assign) NSDate *ftime;
@property (nonatomic, assign) NSCalendarUnit repeat;
@property (nonatomic, assign) NotificationClass *caller;
@property (nonatomic, assign) NSString *customData;
@property (nonatomic, assign) BOOL launched;
-(id)init:(NotificationClass*)caller;
- (void) createNotification;


@end
