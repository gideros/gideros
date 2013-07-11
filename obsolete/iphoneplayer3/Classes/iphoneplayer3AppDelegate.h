//
//  iphoneplayer3AppDelegate.h
//  iphoneplayer3
//
//  Created by Atilim Cetin on 3/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class iphoneplayer3ViewController;

@interface iphoneplayer3AppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    iphoneplayer3ViewController *viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet iphoneplayer3ViewController *viewController;

@end

