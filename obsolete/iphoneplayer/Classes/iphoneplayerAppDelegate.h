//
//  iphoneplayerAppDelegate.h
//  iphoneplayer
//
//  Created by Atilim Cetin on 2/28/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <UIKit/UIKit.h>

@class EAGLView;

@interface iphoneplayerAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    EAGLView *glView;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet EAGLView *glView;

@end

