//
//  main.m
//  MacPlayer
//
//  Created by nico on 28/09/2017.
//  Copyright © 2017 Gideros Mobile. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "AppDelegate.h"

int main(int argc, const char * argv[]) {
    
    NSApplication *app=[NSApplication sharedApplication];
    AppDelegate *appD=[AppDelegate new];
    [app setDelegate:appD];

    return NSApplicationMain (argc, argv);
}
