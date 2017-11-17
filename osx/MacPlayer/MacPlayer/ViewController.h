//
//  ViewController.h
//
//  Copyright 2012 Gideros Mobile. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#define UIView NSView
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>

#import "EAGLView.h"

@interface ViewController : NSViewController
{
    NSOpenGLContext *context;    
    BOOL animating;
    NSInteger animationFrameInterval;
    CVDisplayLinkRef dlink;
}

@property (nonatomic, retain) NSOpenGLContext *context;
@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;
@property (nonatomic, retain) EAGLView* glView;

- (void)startAnimation;
- (void)stopAnimation;

@end
