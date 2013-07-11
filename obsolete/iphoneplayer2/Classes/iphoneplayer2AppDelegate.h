//
//  iphoneplayer2AppDelegate.h
//  iphoneplayer2
//
//  Created by Atilim Cetin on 12/7/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class iphoneplayer2ViewController;

@interface iphoneplayer2AppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    iphoneplayer2ViewController *viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet iphoneplayer2ViewController *viewController;

@end

