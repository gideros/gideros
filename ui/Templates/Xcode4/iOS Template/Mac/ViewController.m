//
//  ViewController.m
//
//  Copyright 2012 Gideros Mobile. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import "ViewController.h"
#import "EAGLView.h"

#define UIView NSView
#include "giderosapi.h"

@interface ViewController ()
@property (nonatomic, assign) CVDisplayLinkRef displayLink;
@end

@implementation ViewController

@synthesize animating, glView, displayLink;

- (id)init
{
	if (self = [super init])
	{
		animating = FALSE;
		animationFrameInterval = 1;
		self.displayLink = nil;		
	}
	
	return self;
}

- (void)loadView
{
    self.glView = [[EAGLView alloc] initWithFrame:CGRectMake(0,0,320,480)];
	self.view = self.glView;
}

- (void)viewDidLoad
{
    [self.glView setup];
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputHandler(displayLink, ^CVReturn(CVDisplayLinkRef  _Nonnull displayLink, const CVTimeStamp * _Nonnull inNow, const CVTimeStamp * _Nonnull inOutputTime, CVOptionFlags flagsIn, CVOptionFlags * _Nonnull flagsOut) {
        dispatch_async(dispatch_get_main_queue(), ^
{
            [self drawFrame];
        });
        return kCVReturnSuccess;
    });
}

- (void)viewWillAppear:(BOOL)animated
{
    [self startAnimation];
}

- (void)viewWillDisappear:(BOOL)animated
{
    [self stopAnimation];
}

- (void)viewDidUnload
{
    CVDisplayLinkRelease(self.displayLink);
    self.displayLink = NULL;
    [self.glView tearDown];
}

- (NSInteger)animationFrameInterval
{
    return animationFrameInterval;
}

- (void)setAnimationFrameInterval:(NSInteger)frameInterval
{
    /*
	 Frame interval defines how many display frames must pass between each time the display link fires.
	 The display link will only fire 30 times a second when the frame internal is two on a display that refreshes 60 times a second. The default frame interval setting of one will fire 60 times a second when the display refreshes at 60 times a second. A frame interval setting of less than one results in undefined behavior.
	 */
    if (frameInterval >= 1)
    {
        animationFrameInterval = frameInterval;
        
        if (animating)
        {
            [self stopAnimation];
            [self startAnimation];
        }
    }
}

- (void)startAnimation
{
    if ((!animating)&&displayLink)
    {
        CVDisplayLinkStart(displayLink);
        
        animating = TRUE;
    }
}

- (void)stopAnimation
{
    if ((animating)&&displayLink)
    {
        CVDisplayLinkStop(displayLink);
        animating = FALSE;
    }
}

- (void)drawFrame
{
    @autoreleasepool {
    	gdr_drawFrame(resized);
	resized=FALSE;
    }
}

- (void)didReceiveMemoryWarning
{
    
    // Release any cached data, images, etc. that aren't in use.
	gdr_didReceiveMemoryWarning();
}

- (void)windowDidResize:(NSNotification *)notification
{
    [self.glView resized];
    resized=TRUE;
}
- (void)windowDidEnterFullScreen:(NSNotification *)notification
{
    resized=TRUE;
    [self.glView resized];
}
- (void)windowDidExitFullScreen:(NSNotification *)notification
{
    resized=TRUE;
    [self.glView resized];
}

- (NSUInteger)supportedInterfaceOrientations
{
    /*NSArray *supportedOrientations = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"UISupportedInterfaceOrientations"];
     NSUInteger result = gdr_supportedInterfaceOrientations();
     BOOL supported = false;
     for (NSNumber *orientation in supportedOrientations) {
     if((result & [orientation integerValue]) != 0){
     supported = true;
     break;
     }
     }
     if(supported)
     return result;
     else
     return UIInterfaceOrientationMaskAll;
    return gdr_supportedInterfaceOrientations();*/
    return 0;
}

@end
