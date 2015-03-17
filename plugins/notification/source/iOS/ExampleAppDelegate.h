//
//  AppDelegate.h
//
//  Copyright 2012 Gideros Mobile. All rights reserved.
//

#import <UIKit/UIKit.h>

@class ViewController;

@interface AppDelegate : NSObject <UIApplicationDelegate>
{
    UIWindow *window;
    ViewController *viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet ViewController *viewController;

@property (nonatomic, retain) UILocalNotification *launchLocalNotification;
@property (nonatomic, retain) NSDictionary *launchRemoteNotification;

@end

