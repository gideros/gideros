//
//  GiderosiPhonePlayerAppDelegate.h
//  GiderosiPhonePlayer
//
//  Created by Atilim Cetin on 6/8/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class GiderosiPhonePlayerViewController;

@interface GiderosiPhonePlayerAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    GiderosiPhonePlayerViewController *viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet GiderosiPhonePlayerViewController *viewController;

@end

