//
//  GiderosiPadPlayerAppDelegate.h
//  GiderosiPadPlayer
//
//  Created by Atilim Cetin on 6/8/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class GiderosiPadPlayerViewController;

@interface GiderosiPadPlayerAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    GiderosiPadPlayerViewController *viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet GiderosiPadPlayerViewController *viewController;

@end

