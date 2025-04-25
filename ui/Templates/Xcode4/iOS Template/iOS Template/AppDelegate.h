//
//  AppDelegate.h
//
//  Copyright 2012 Gideros Mobile. All rights reserved.
//

#import <UIKit/UIKit.h>

@class ViewController;

@interface AppDelegate : NSObject <UIApplicationDelegate, UIWindowSceneDelegate>
{
    UIWindow *window;
    ViewController *viewController;
}

@property (nonatomic, retain, nullable) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet ViewController *viewController;

@end

