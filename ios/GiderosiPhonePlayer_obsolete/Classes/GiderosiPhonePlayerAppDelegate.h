//
//  GiderosiPhonePlayerAppDelegate.h
//  GiderosiPhonePlayer
//
//  Created by Atilim Cetin on 1/16/12.
//  Copyright 2012 Gideros Mobile. All rights reserved.
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

