//
//  ViewController.h
//
//  Copyright 2012 Gideros Mobile. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <CoreVideo/CoreVideo.h>
#import "EAGLView.h"

@interface ViewController : NSViewController<NSWindowDelegate>
{
    NSTableView *tableView;
	    
    BOOL animating;
    BOOL resized;
    NSInteger animationFrameInterval;
    CVDisplayLinkRef displayLink;

	EAGLView* glView;
}

@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic, getter=getGlView) EAGLView *glView;
@property (nonatomic) NSInteger animationFrameInterval;

- (void)startAnimation;
- (void)stopAnimation;
- (void)windowDidResize:(NSNotification *)notification;

@end
