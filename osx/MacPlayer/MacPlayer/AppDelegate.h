//
//  AppDelegate.h
//
//  Copyright 2012 Gideros Mobile. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class ViewController;

@interface AppDelegate : NSObject <NSApplicationDelegate>
{
    NSWindow *window;
    ViewController *viewController;
}
- (void) applicationWillFinishLaunching: (NSNotification *)not;

@property (nonatomic, retain) NSWindow *window;
@property (nonatomic, retain) ViewController *viewController;

@end

