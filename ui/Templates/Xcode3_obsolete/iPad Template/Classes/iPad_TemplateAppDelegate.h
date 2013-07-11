//
//  iPad_TemplateAppDelegate.h
//  iPad Template
//

#import <UIKit/UIKit.h>

@class iPad_TemplateViewController;

@interface iPad_TemplateAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    iPad_TemplateViewController *viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet iPad_TemplateViewController *viewController;

@end

