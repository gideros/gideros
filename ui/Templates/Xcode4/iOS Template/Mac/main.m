//
//  main.m
//  Player Mac
//
//  Created by nico on 25/11/2020.
//  Copyright © 2020 Gideros Mobile. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "AppDelegate.h"
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
          [NSApplication sharedApplication];

          AppDelegate *appDelegate = [[AppDelegate alloc] init];
          [NSApp setDelegate:appDelegate];
          [NSApp run];
    }
}
