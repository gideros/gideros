//
//  ViewController.m
//
//  Copyright 2012 Gideros Mobile. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import "ViewController.h"
#import "EAGLView.h"

#include "giderosapi.h"

@implementation ViewController

@synthesize animating, context, glView;

- (id)init
{
	if (self = [super init])
	{
		animating = FALSE;
		animationFrameInterval = 1;
        CVDisplayLinkCreateWithActiveCGDisplays(&dlink);
        CVDisplayLinkSetOutputHandler(dlink, ^CVReturn (CVDisplayLinkRef displayLink, const CVTimeStamp * _Nonnull inNow, const CVTimeStamp * _Nonnull inOutputTime, CVOptionFlags flagsIn, CVOptionFlags * _Nonnull flagsOut) {
            [self drawFrame];
            return kCVReturnSuccess;
        });
	}
	
	return self;
}

- (void)loadView
{
    self.glView = [[EAGLView alloc] init];
	//self.glView.clearsContextBeforeDrawing = NO;
	//self.glView.multipleTouchEnabled = YES;
    self.glView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
	
	NSView* rootView = [[NSView alloc] init];
	[rootView addSubview:glView];   
	
	self.view = rootView;
}

- (void) getSafeArea:(CGRect *) sa 
{
 	*sa=CGRectMake(0,0,0,0);
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
    if (!animating)
    {
        [glView.context makeCurrentContext];
        CVDisplayLinkStart(dlink);
        animating = TRUE;
    }
}

- (void)stopAnimation
{
    if (animating)
    {
        CVDisplayLinkStop(dlink);
        animating = FALSE;
    }
}

- (void)drawFrame
{
    dispatch_async( dispatch_get_main_queue(), ^{
        [glView.context makeCurrentContext];
        gdr_drawFrame();
    });
}

@end
