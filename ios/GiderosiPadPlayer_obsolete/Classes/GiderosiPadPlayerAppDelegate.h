//
//  GiderosiPadPlayerAppDelegate.h
//  GiderosiPadPlayer
//
//  Created by Atilim Cetin on 1/18/12.
//  Copyright 2012 Gideros Mobile. All rights reserved.
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

