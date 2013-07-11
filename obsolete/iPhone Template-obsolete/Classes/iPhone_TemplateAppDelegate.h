//
//  iPhone_TemplateAppDelegate.h
//  iPhone Template
//
//  Created by Atilim Cetin on 6/8/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class iPhone_TemplateViewController;

@interface iPhone_TemplateAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    iPhone_TemplateViewController *viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet iPhone_TemplateViewController *viewController;

@end

