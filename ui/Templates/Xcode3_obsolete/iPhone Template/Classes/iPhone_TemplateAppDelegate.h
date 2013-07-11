//
//  iPhone_TemplateAppDelegate.h
//  iPhone Template
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

