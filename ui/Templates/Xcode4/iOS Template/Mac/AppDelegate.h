//
//  AppDelegate.h
//  Player Mac
//
//  Created by nico on 25/11/2020.
//  Copyright © 2020 Gideros Mobile. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class ViewController;

@interface AppDelegate : NSObject <NSApplicationDelegate>
{
    NSWindow *window;
    ViewController *viewController;
}

@property (nonatomic, retain) IBOutlet NSWindow *window;
@property (nonatomic, retain) IBOutlet ViewController *viewController;

@end

