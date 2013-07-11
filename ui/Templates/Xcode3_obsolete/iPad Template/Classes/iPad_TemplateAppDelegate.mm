//
//  iPad_TemplateAppDelegate.mm
//  iPad Template
//

#import "iPad_TemplateAppDelegate.h"
#import "iPad_TemplateViewController.h"

#include "../giderosapi.h"

@implementation iPad_TemplateAppDelegate

@synthesize window;
@synthesize viewController;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	CGRect bounds = [[UIScreen mainScreen] bounds];
	
    self.window = [[[UIWindow alloc] initWithFrame:bounds] autorelease];
	
    self.viewController = [[[iPad_TemplateViewController alloc] init] autorelease];	
    self.viewController.wantsFullScreenLayout = YES;
	
	[self.viewController view];
	
	gdr_initialize(self.viewController.glView, bounds.size.width, bounds.size.height, false);

	[self.window addSubview:self.viewController.view];
    [self.window makeKeyAndVisible];
		
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
	gdr_suspend();
    [self.viewController stopAnimation];
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
	gdr_resume();
    [self.viewController startAnimation];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	gdr_exitGameLoop();
    [self.viewController stopAnimation];
	gdr_deinitialize();
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Handle any background procedures not related to animation here.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Handle any foreground procedures not related to animation here.
}

- (void)dealloc
{
    [viewController release];
    [window release];
    
    [super dealloc];
}

@end
